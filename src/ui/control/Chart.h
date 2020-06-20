#ifndef CHART_H
#define CHART_H

#include <cstdint>
#include <algorithm>

#include <LovyanGFX.h>

#include "DrawDefs.h"

/**
 * @brief X軸の描画設定
 */
enum class ChartMode : uint32_t {
    Overwrite, /**< 最初の位置に戻って最初のデータの上に上書き */
    Scroll, /**< 全体的に左にシフトしてから新しいデータを表示する */
    Infinite, /**< 過去のデータを圧縮して新しいデータが常に表示されるようにする */
};

/**
 * @brief  Y軸の設定
 */
struct AxisY {
    float min; /**< 最小値 */
    float max; /**< 最大値 */
};

/**
 * @brief Chartの描画設定
 */
struct ChartConfig {
    ChartMode mode; /**< 描画設定 */
    Rect  rect; /**< 表示位置とサイズ */
    AxisY axisY0; /**< 左側のY軸設定 */
    AxisY axisY1; /**< 左側のY軸設定 */
    Color axisColor; /**< 軸の色 */
    uint32_t axisTickness; /**< 軸の太さ */
    Color backColor; /**< 背景色 */
};

/**
 * @brief Chartの点追加時の描画設定です
 * @note Chart classで任意数のデータ系列を扱うため、内部で保持せず外部で指定する方式にしています
 */
struct PlotConfig {
    uint32_t axisYIndex; /**< Y軸のIndex、左なら0、右なら1を指定(それ以上に軸を増やす実装は未対応) */
    Color color; /**< 色 */
};

/**
 * @brief LovyanGFXを使ってチャートを描画する機能を提供します。
 * @note このクラスは描画を行うのみで履歴値は保持しません。点数を大きくしてもリソースを圧迫しません
 */
class Chart {
    public:
        /**
         * @brief Construct a new Chart object
         */
        Chart(void) {}

        /**
         * @brief Destroy the Chart object
         */
        virtual ~Chart(void) {}

        /**
         * @brief グラフ描画設定を初期化します
         * 
         * @param config 描画設定
         * @param drawDst 描画先lcd or offscreen bufferを指定します
        *
         * @retval true 描画完了
         * @retval false 描画失敗
         */
        bool init(LovyanGFX& drawDst, const ChartConfig& config) {
            // release buffer
            if (this->isInitialized) {
                this->isInitialized = false;
            }
            // config validation
            if (config.rect.width == 0) return false;
            if (config.rect.height == 0) return false;

            // set variables
            this->config = config;
            this->xIndex = 0;

            // 背景準備
            this->drawBackground(drawDst);
            this->drawAxis(drawDst);

            // 設定完了
            this->isInitialized = true;
            return true;
        }

        /**
         * @brief 点を追加します
         * @remark 一通りの系列データをplotし終わったらflush()を呼び出してX軸位置をincrementしてください
         * 
         * @param y 最新値
         * @param plotConfig 描画設定
         * @param drawDst 描画先lcd or offscreen bufferを指定します
         */
        void plot(LovyanGFX& drawDst, float y, const PlotConfig& plotConfig) {
            // 未初期化なら失敗
            if (!this->isInitialized) {
                return;
            }
            // TODO: Infinite/Scroll対応

            // 描画位置計算
            const float minY   = (plotConfig.axisYIndex == 0) ? this->config.axisY0.min : this->config.axisY1.min;
            const float maxY   = (plotConfig.axisYIndex == 0) ? this->config.axisY0.max : this->config.axisY1.max;
            const float areaY  = (maxY - minY);
            if (areaY == 0.0f) return;
            const float ratioY = (y - minY) / areaY;
            if (ratioY < 0.0f || 1.0f < ratioY) return;
            // ratioYに0.0f~1.0fが入っているので描画領域からY座標を推定
            const uint32_t plotY = (this->getPlotOffsetY0() + this->getPlotHeight()) - static_cast<uint32_t>(ratioY * this->getPlotHeight());

            // 点を打つ
            const auto plotColor =
                drawDst.color888(
                    plotConfig.color.r, 
                    plotConfig.color.g, 
                    plotConfig.color.b
                    );
            const uint32_t plotX = this->getPlotOffsetX0() + this->xIndex;
            drawDst.drawPixel(plotX, plotY, plotColor);

        }

        /**
         * @brief Xのデータ位置を進め、plot内容を確定させます
         * 
         * @param drawDst 描画先lcd or offscreen bufferを指定します
         */
        void next(LovyanGFX& drawDst) {
            // 未初期化なら失敗
            if (!this->isInitialized) {
                return;
            }
            // x indexをすすめる
            switch (this->config.mode) {
                case ChartMode::Overwrite:
                    // 全部描画したら最初に戻る
                    this->xIndex = (this->xIndex + 1) % this->getPlotWidth();
                    break;
                case ChartMode::Scroll:
                case ChartMode::Infinite:
                    // いずれも一番最後の領域に描く
                    // 差分は0~n-1のSpriteをシフトするか圧縮するかの差
                    this->xIndex = std::min(this->xIndex + 1, this->getPlotWidth() - 1);
                    break;
                default:
                    // 未実装
                    break;
            }
            // 予め塗りつぶしておく
            const auto backColor =
                drawDst.color888(
                    this->config.backColor.r, 
                    this->config.backColor.g, 
                    this->config.backColor.b
                    );

            // 描画領域初期化
            drawDst.fillRect(
                this->getPlotOffsetX0() + this->xIndex,
                this->getPlotOffsetY0(),
                1,
                this->getPlotHeight(),
                backColor
                );

            // TODO: 初期化時に全部かけるように関数に切り出す
            // 軸の点線
            if (this->xIndex % 20 == 0) { // TODO: configへ
                const PlotConfig y0Config = {
                    .axisYIndex = 0,
                    .color = {
                        .r = this->config.axisColor.r, 
                        .g = this->config.axisColor.g, 
                        .b = this->config.axisColor.b
                    },
                };
                const PlotConfig y1Config = {
                    .axisYIndex = 1,
                    .color = {
                        .r = this->config.axisColor.r, 
                        .g = this->config.axisColor.g, 
                        .b = this->config.axisColor.b
                    },
                };

                const uint32_t n = 5; // TODO: configへ
                for (uint32_t i = 0; i < n; i ++) {  
                    const float ratio = static_cast<float>(i) / static_cast<float>(n);
                    const float y0 =  ratio * (this->config.axisY0.max - this->config.axisY0.min) + this->config.axisY0.min;
                    const float y1 =  ratio * (this->config.axisY1.max - this->config.axisY1.min) + this->config.axisY1.min;
                    this->plot(drawDst, y0, y0Config);
                    this->plot(drawDst, y1, y1Config);
                }
            }
        }

    protected:
        // local variables
        bool isInitialized; /**< initが呼ばれていなければfalse */
        ChartConfig config; /**< 描画設定 */
        uint32_t xIndex; /**< X軸のデータ位置 */

        /**
         * @brief 背景を塗りつぶします
         * 
         * @param drawDst 描画先
         */
        void drawBackground(LovyanGFX& drawDst) {
            // 背景塗りつぶし用
            const auto backColor =
                drawDst.color888(
                    this->config.backColor.r, 
                    this->config.backColor.g, 
                    this->config.backColor.b
                    );

            // 描画領域初期化
            drawDst.fillRect(
                this->config.rect.x,
                this->config.rect.y,
                this->config.rect.width,
                this->config.rect.height,
                backColor
                );
        }
        /**
         * @brief 軸を描画します
         * 
         * @param drawDst 描画先
         */
        void drawAxis(LovyanGFX& drawDst) {
            const auto axisColor = 
                drawDst.color888(
                    this->config.axisColor.r, 
                    this->config.axisColor.g, 
                    this->config.axisColor.b
                    );
            for (uint32_t t = 0; t < this->config.axisTickness; t++) {
                // top
                drawDst.drawLine(
                    this->config.rect.x,
                    this->config.rect.y + t,
                    this->config.rect.x + this->config.rect.width,
                    this->config.rect.y + t,
                    axisColor
                    );
                // bottom
                drawDst.drawLine(
                    this->config.rect.x,
                    this->config.rect.y + this->config.rect.height - t,
                    this->config.rect.x + this->config.rect.width,
                    this->config.rect.y + this->config.rect.height - t,
                    axisColor
                    );
                // left
                drawDst.drawLine(
                    this->config.rect.x + t,
                    this->config.rect.y,
                    this->config.rect.x + t,
                    this->config.rect.y + this->config.rect.height,
                    axisColor
                    );
                // right
                drawDst.drawLine(
                    this->config.rect.x + this->config.rect.width - t,
                    this->config.rect.y,
                    this->config.rect.x + this->config.rect.width - t,
                    this->config.rect.y + this->config.rect.height,
                    axisColor
                    );
            }
        }
        /**
         * @brief 描画領域の横幅を取得します
         * 
         * @return constexpr uint32_t 
         */
        constexpr uint32_t getPlotWidth(void) {
            return this->config.rect.width - this->config.axisTickness * 2;
        }

        /**
         * @brief 描画領域の高さを取得します
         * 
         * @return constexpr uint32_t 
         */
        constexpr uint32_t getPlotHeight(void) {
            return this->config.rect.height - this->config.axisTickness * 2;
        }

        /**
         * @brief グラフ描画位置の左上を取得します
         * 
         * @return constexpr uint32_t 
         */
        constexpr uint32_t getPlotOffsetX0(void) {
            return this->config.rect.x + this->config.axisTickness;
        }

        /**
         * @brief グラフ描画位置の左上を取得します
         * 
         * @return constexpr uint32_t 
         */
        constexpr uint32_t getPlotOffsetY0(void) {
            return this->config.rect.y + this->config.axisTickness;
        }


};

#endif /* CHART_H */
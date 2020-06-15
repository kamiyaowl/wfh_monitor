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
        *
         * @retval true 描画完了
         * @retval false 描画失敗
         */
        bool init(const ChartConfig& config) {
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
            this->latestPlotXIndex = UINT32_MAX; // 初回にplotされたときにSprite準備が走るように

            // 設定完了
            this->isInitialized = true;
            this->isDirty = false;
            return true;
        }

        /**
         * @brief 点を追加します
         * @remark 一通りの系列データをplotし終わったらflush()を呼び出してX軸位置をincrementしてください
         * 
         * @param y 最新値
         * @param plotConfig 描画設定
         */
        void plot(float y, const PlotConfig& plotConfig) {
            // 未初期化なら失敗
            if (!this->isInitialized) {
                return;
            }
            // 初めて現在のX位置で描画された際のSprite準備
            if (this->xIndex != this->latestPlotXIndex) {
                this->latestPlotXIndex = this->xIndex;
                // TODO: Sprite準備
            }
            this->isDirty = true; // flush時に昔のゴミを上書きしないための対策
        }

        /**
         * @brief Xのデータ位置を進め、plot内容を確定させます
         */
        void flush(void) {
            // 未初期化なら失敗
            if (!this->isInitialized) {
                return;
            }
            if (this->isDirty) {
                this->isDirty = false;
            }
            // x indexをすすめる
            switch (this->config.mode) {
                case ChartMode::Overwrite:
                    // 全部描画したら最初に戻る
                    this->xIndex = (this->xIndex + 1) % this->config.rect.width;
                    break;
                case ChartMode::Scroll:
                case ChartMode::Infinite:
                    // いずれも一番最後の領域に描く
                    // 差分は0~n-1のSpriteをシフトするか圧縮するかの差
                    this->xIndex = std::min(this->xIndex + 1, this->config.rect.width);
                    break;
                default:
                    // 未実装
                    break;
            }
        }

        /**
         * @brief 引数で指定されたLCDもしくはSpriteに描画します
         * 
         * @param drawDst 描画先lcd or offscreen bufferを指定します
         */
        void draw(LovyanGFX& drawDst) {
            // 未初期化なら失敗
            if (!this->isInitialized) {
                return;
            }

            // 軸を描く
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
    protected:
        // local variables
        bool isInitialized; /**< initが呼ばれていなければfalse */
        bool isDirty; /**< 今回のxIndexでplotが一度でもされたらtrue */
        ChartConfig config; /**< 描画設定 */
        uint32_t xIndex; /**< X軸のデータ位置 */
        uint32_t latestPlotXIndex; /**< 最後にPlotを行ったX軸のデータ位置 */

        /**
         * @brief 描画領域の横幅を取得します
         * 
         * @return constexpr uint32_t 
         */
        constexpr uint32_t getPlotWidth(void) {
            const uint32_t y0 = this->config.axisTickness;
            const uint32_t y1 = this->config.axisTickness;
            return this->config.rect.width - (y0 + y1);
        }

        /**
         * @brief 描画領域の高さを取得します
         * 
         * @return constexpr uint32_t 
         */
        constexpr uint32_t getPlotHeight(void) {
            const uint32_t x0 = this->config.axisTickness;
            const uint32_t x1 = this->config.axisTickness;
            return this->config.rect.height - (x0 + x1);
        }

        /**
         * @brief config.rect.xに対するグラフ描画位置の左上を取得します
         * 
         * @return constexpr uint32_t 
         */
        constexpr uint32_t getPlotOffsetX0(void) {
            return this->config.axisTickness;
        }

        /**
         * @brief config.rect.yに対するグラフ描画位置の左上を取得します
         * 
         * @return constexpr uint32_t 
         */
        constexpr uint32_t getPlotOffsetY0(void) {
            return this->config.axisTickness;
        }


};

#endif /* CHART_H */
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
 * @brief X軸の設定
 */
struct AxisX {
    uint32_t n; /**< X軸の表示点数 */
    bool isVisible; /**< 軸を非表示にする場合はfalse */
};

/**
 * @brief  Y軸の設定
 */
struct AxisY {
    float min; /**< 最小値 */
    float max; /**< 最大値 */
    bool isVisible; /**< 軸を非表示にする場合はfalse */
};

/**
 * @brief Chartの描画設定
 */
struct ChartConfig {
    ChartMode mode; /**< 描画設定 */
    Rect  rect; /**< 表示位置とサイズ */
    AxisX axisX; /**< X軸設定 */
    AxisY axisY0; /**< 左側のY軸設定 */
    AxisY axisY1; /**< 左側のY軸設定 */
    Color axisColor; /**< 軸の色 */
    uint32_t axisTickness; /**< 軸の太さ */
};

/**
 * @brief Chartの点追加時の描画設定です
 * @note Chart classで任意数のデータ系列を扱うため、内部で保持せず外部で指定する方式にしています
 */
struct PlotConfig {
    uint32_t axisYIndex; /**< Y軸のIndex、左なら0、右なら1を指定(それ以上に軸を増やす実装は未対応) */
    uint32_t r; /**< 点の大きさ */
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
                // TODO: Spriteを一旦開放する
            }
            // config validation
            if (config.axisX.n == 0) return false;
            if (config.rect.width == 0) return false;
            if (config.rect.height == 0) return false;

            // set variables
            this->config = config;
            this->xIndex = 0;
            this->latestPlotXIndex = UINT32_MAX; // 初回にplotされたときにSprite準備が走るように

            // initialize sprite
            // TODO: Spriteを確保する, できなければfalseで返す

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
         * 
         * @retval true 描画完了
         * @retval false 描画失敗
         */
        bool plot(float y, const PlotConfig& plotConfig) {
            // 未初期化なら失敗
            if (!this->isInitialized) {
                return false;
            }
            // 初めて現在のX位置で描画された際のSprite準備
            if (this->xIndex != this->latestPlotXIndex) {
                this->latestPlotXIndex = this->xIndex;
                // TODO: Sprite準備
            }
            // TODO: spriteCurrentDivisionにPlotする
            this->isDirty = true; // flush時に昔のゴミを上書きしないための対策
        }

        /**
         * @brief Xのデータ位置を進め、plot内容を確定させます
         */
        void flush(void) {
            // 未初期化なら失敗
            if (!this->isInitialized) {
                return false;
            }
            // TODO: sritePlotAreaにspriteCurrentDivisionの内容を確定させる
            if (this->isDirty) {
                this->isDirty = false;

            }
            // x indexをすすめる
            switch (this->config.mode) {
                case ChartMode::Overwrite:
                    // 全部描画したら最初に戻る
                    this->xIndex = (this->xIndex + 1) % this->config.axisX.n;
                    break;
                case ChartMode::Scroll:
                case ChartMode::Infinite:
                    // いずれも一番最後の領域に描く
                    // 差分は0~n-1のSpriteをシフトするか圧縮するかの差
                    this->xIndex = std::min(this->xIndex + 1, this->config.axisX.n);
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
         * @param isInit 初回、もしくは別画面からの遷移でフルで描画が必要な場合はtrue, falseの場合はY軸の描画などは省略されます
         */
        void draw(LovyanGFX& drawDst, bool isInit) {
            // 未初期化なら失敗
            if (!this->isInitialized) {
                return false;
            }
            // 全描画が必要な場合
            if (isInit) {
                // TODO: Y0, Y1 軸を描く
            }
            // TODO: spritePlotAreaの内容を反映させる
        }
    protected:
        // Buffer
        LGFX_Sprite spritePlotArea; /**< X軸を含むPlotArea */
        LGFX_Sprite spriteAxisY0; /**< 左側のY軸 */
        LGFX_Sprite spriteAxisY1; /**< 右側のY軸 */
        LGFX_Sprite spriteCurrentDivision; /**< spritePlotAreaの内、今回描画しようとしている */
        LGFX_Sprite spritePlotAreaTemp;
        // local variables
        bool isInitialized; /**< initが呼ばれていなければfalse */
        bool isDirty; /**< 今回のxIndexでplotが一度でもされたらtrue */
        ChartConfig config; /**< 描画設定 */
        uint32_t xIndex; /**< X軸のデータ位置 */
        uint32_t latestPlotXIndex; /**< 最後にPlotを行ったX軸のデータ位置 */

};

#endif /* CHART_H */
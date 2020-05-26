#ifndef IPCQUEUEDEFS_H
#define IPCQUEUEDEFS_H

#include <cstdint>

/**
 * @brief 測定データ
 */
typedef struct {
    float visibleLux;   /**< 明るさセンサの値 */
    float tempature;    /**< 温度センサの値 */
    float pressure;     /**< 気圧センサの値 */
    float humidity;     /**< 湿度センサの値 */
    float gas;          /**< ガスセンサの値 */
    uint32_t timestamp; /**< for debug */
} MeasureData_t;

namespace ButtonState {
    static constexpr uint32_t UP    = 0x00000001;
    static constexpr uint32_t DOWN  = 0x00000002;
    static constexpr uint32_t LEFT  = 0x00000004;
    static constexpr uint32_t RIGHT = 0x00000008;
    static constexpr uint32_t PRESS = 0x00000010;
    static constexpr uint32_t A     = 0x00000020;
    static constexpr uint32_t B     = 0x00000040;
    static constexpr uint32_t C     = 0x00000080;
}

/**
 * @brief ボタン入力情報
 * @note bmpの割当はButtonState以下の定義に従う
 */
typedef struct {
    uint32_t raw;       /**< 現在の入力値 */
    uint32_t debounce;  /**< チャタリング除去済の値 */
    uint32_t push;      /**< debounceの内、release->push変化した値 */
    uint32_t release;   /**< debounceの内、push->release変化した値 */
} ButtonStateBmp_t;

#endif /* IPCQUEUEDEFS_H */
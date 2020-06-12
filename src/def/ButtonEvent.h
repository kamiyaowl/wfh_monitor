#ifndef BUTTONEVENT_H
#define BUTTONEVENT_H

#include <cstdint>

/**
 * @brief ボタン入力値の定義
 */
enum class ButtonState : uint32_t {
    None  = 0x00000000,
    Up    = 0x00000001,
    Down  = 0x00000002,
    Left  = 0x00000004,
    Right = 0x00000008,
    Press = 0x00000010,
    A     = 0x00000020,
    B     = 0x00000040,
    C     = 0x00000080,
};

/**
 * @brief ボタン入力情報
 * @note bmpの割当はButtonState以下の定義に従う
 */
struct ButtonEventData {
    uint32_t raw;       /**< 現在の入力値 */
    uint32_t debounce;  /**< チャタリング除去済の値 */
    uint32_t push;      /**< debounceの内、release->push変化した値 */
    uint32_t release;   /**< debounceの内、push->release変化した値 */
    uint32_t timestamp;
};


#endif /* BUTTONEVENT_H */
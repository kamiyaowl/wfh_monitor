#include "SysTimer.h"
#include "ButtonTask.h"

#ifdef WFH_MONITOR_ENABLE_SERIAL_PRINT_BUTTON_DATA
/**
 * @brief Arduinoのシリアルプロッタ用にボタン入力の値を出力します
 * 
 * @param serial Serial Peripheral
 * @param data 測定したButton Data Data
 */
static void debugSerialPrint(Serial_& serial, const uint32_t& data) {
    serial.print((data & ButtonState::UP)    != 0x0);
    serial.print(",");
    serial.print((data & ButtonState::DOWN)  != 0x0);
    serial.print(",");
    serial.print((data & ButtonState::LEFT)  != 0x0);
    serial.print(",");
    serial.print((data & ButtonState::RIGHT) != 0x0);
    serial.print(",");
    serial.print((data & ButtonState::PRESS) != 0x0);
    serial.print(",");
    serial.print((data & ButtonState::A)     != 0x0);
    serial.print(",");
    serial.print((data & ButtonState::B)     != 0x0);
    serial.print(",");
    serial.print((data & ButtonState::C)     != 0x0);
    serial.println(",");
}
#endif /* WFH_MONITOR_ENABLE_SERIAL_PRINT_BUTTON_DATA */

void ButtonTask::setup(void) {
    // fps control
    this->setFps(60);

    // port initialize
    pinMode(WIO_5S_UP,    INPUT_PULLUP);
    pinMode(WIO_5S_DOWN,  INPUT_PULLUP);
    pinMode(WIO_5S_LEFT,  INPUT_PULLUP);
    pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
    pinMode(WIO_5S_PRESS, INPUT_PULLUP);
    pinMode(WIO_KEY_A,    INPUT_PULLUP);
    pinMode(WIO_KEY_B,    INPUT_PULLUP);
    pinMode(WIO_KEY_C,    INPUT_PULLUP);

    // variable initialize
    this->oldDebounce = ButtonState::NONE;
}

bool ButtonTask::loop(void) {
    // get raw button input
    uint32_t raw = 0x0;
    raw |= (digitalRead(WIO_5S_UP)    == LOW) ? ButtonState::UP    : ButtonState::NONE;
    raw |= (digitalRead(WIO_5S_DOWN)  == LOW) ? ButtonState::DOWN  : ButtonState::NONE;
    raw |= (digitalRead(WIO_5S_LEFT)  == LOW) ? ButtonState::LEFT  : ButtonState::NONE;
    raw |= (digitalRead(WIO_5S_RIGHT) == LOW) ? ButtonState::RIGHT : ButtonState::NONE;
    raw |= (digitalRead(WIO_5S_PRESS) == LOW) ? ButtonState::PRESS : ButtonState::NONE;
    raw |= (digitalRead(WIO_KEY_A)    == LOW) ? ButtonState::A     : ButtonState::NONE;
    raw |= (digitalRead(WIO_KEY_B)    == LOW) ? ButtonState::B     : ButtonState::NONE;
    raw |= (digitalRead(WIO_KEY_C)    == LOW) ? ButtonState::C     : ButtonState::NONE;

    // TODO: debounce
    const uint32_t currentDebounce = raw;

    // edge detect
    const uint32_t push    = currentDebounce   & (currentDebounce ^ this->oldDebounce); // 差分かつ今回いるもの
    const uint32_t release = this->oldDebounce & (currentDebounce ^ this->oldDebounce); // 差分かつ前回いるもの

    // send Queue
    const ButtonStateBmp_t data = {
        .raw = raw,
        .debounce = currentDebounce,
        .push = push,
        .release = release,
        .timestamp = SysTimer::getTickCount(),
    };
    if (this->sendQueue.emptyNum() > 0) {
        this->sendQueue.send(&data);
    }
    // update oldDebounce
    this->oldDebounce = currentDebounce;

#ifdef WFH_MONITOR_ENABLE_SERIAL_PRINT_BUTTON_DATA
    debugSerialPrint(this->serial, currentDebounce);
#endif /* WFH_MONITOR_ENABLE_SERIAL_PRINT_BUTTON_DATA */

    return false; /**< no abort */
}

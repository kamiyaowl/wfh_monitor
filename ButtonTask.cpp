#include "ButtonTask.h"

void ButtonTask::setup(void) {
    this->serial.println("ButtonTask setup");
}

bool ButtonTask::loop(void) {
    this->serial.println("ButtonTask loop");
    //TODO: ボタン入力と平均化、トリガ検出を実装

    return false; /**< no abort */
}

#include "ButtonTask.h"

void ButtonTask::setup(void) {
    this->serial.println("ButtonTask setup");
}

bool ButtonTask::loop(void) {
    this->serial.println("ButtonTask loop");
    delay(100);
    //TODO:

    return false; /**< no abort */
}

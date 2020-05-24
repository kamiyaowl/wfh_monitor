#include "UiTask.h"

void UiTask::setup(void) {
    this->serial.println("UiTask setup");
    this->lcd.init();
}

bool UiTask::loop(void) {
    this->serial.println("UiTask loop");
    //TODO: いい感じのUI
    this->lcd.setCursor(0,0);
    this->lcd.printf("counter=%d\n", this->counter);


    /* for debug */
    this->counter++;

    return false; /**< no abort */
}

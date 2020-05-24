#include "GroveTask.h"

void GroveTask::setup(void) {
    this->serial.println("GroveTask setup");
}

bool GroveTask::loop(void) {
    this->serial.println("GroveTask loop");
    delay(1000);

    return false;
}

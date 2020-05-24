#include "GroveTask.h"

void GroveTask::setup(void) {
    this->serial.println("GroveTask setup");
}

void GroveTask::loop(void) {
    this->serial.println("GroveTask loop");
    delay(1000);
}

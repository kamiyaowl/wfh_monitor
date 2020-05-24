#include "TaskBase.h"

template<typename T>
TaskBase<T>::TaskBase(void){
}

template<typename T>
TaskBase<T>::~TaskBase() {
    this->deleteTask();
}

template<typename T>
void TaskBase<T>::createTask(size_t stackSize, uint32_t priority, T param) {
    /* already running */
    if (this->isRunning) return;

    this->isRunning = true;
    this->param = param;
    xTaskCreate(
        [](void* pvParameter){
            TaskBase* this_ptr = static_cast<TaskBase*>(pvParameter);
            this_ptr->run();
        },
        this->getName(),
        stackSize,
        this,
        priority,
        &this->taskHandle
    );
}

template<typename T>
void TaskBase<T>::deleteTask(void) {
    /* task is not running */
    if (!this->isRunning) return;

    this->isRunning = false;
    vTaskDelete(this->taskHandle);
}

template<typename T>
void TaskBase<T>::run(void) {
    setup();
    while(true) {
        loop();
    }
}

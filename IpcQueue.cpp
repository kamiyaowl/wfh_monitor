#include "IpcQueue.h"

template<typename T>
bool IpcQueue<T>::createQueue(size_t queueDepth) {
    // invalid operation
    if (queueDepth == 0) return false;
    // already created
    if (this->isInitialized) return false;

    // create from rtos api
    this->queueHandle = xQueueCreate(queueDepth, this->entrySize);
    if (this->queueHandle == nullptr) return false;

    this->depth = queueDepth;
    this->isInitialized = true;
    return true;
}

template<typename T>
bool IpcQueue<T>::deleteQueue(void) {
    // not created
    if (!this->isInitialized) return true;
    
    vQueueDelete(this->queueHandle);

    this->depth = 0;
    this->isInitialized = false;
}

template<typename T>
bool IpcQueue<T>::reset(void) {
    // not created
    if (!this->isInitialized) return true;

    const auto result = xQueueReset(this->queueHandle);
    return (result == pdTRUE);
}

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

template<typename T>
bool IpcQueue<T>::send(T value) {
    // not created
    if (!this->isInitialized) return false;

    const auto result = xQueueSend(this->queueHandle, value, 0);
    return (result == pdPASS);
}

template<typename T>
bool IpcQueue<T>::receive(T* value) {
    // not created
    if (!this->isInitialized) return false;
    // invalid dst ptr
    if (value == nullptr) return false;

    const auto result = xQueueReceive(this->queueHandle, value, portMAX_DELAY); /**< Messageが貯まるまで待つ*/
    return (result == pdPASS);
}

template<typename T>
size_t IpcQueue<T>::remainNum(void) {
    // not created
    if (!this->isInitialized) return 0;

    return uxQueueMessagesWaiting(this->queueHandle);
}

template<typename T>
size_t IpcQueue<T>::emptyNum(void) {
    // not created
    if (!this->isInitialized) return 0;

    return this->depth - uxQueueMessagesWaiting(this->queueHandle);
}


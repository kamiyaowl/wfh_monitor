#ifndef IPCQUEUE_H
#define IPCQUEUE_H

#include <cstdint>

#include <Seeed_Arduino_FreeRTOS.h>

template<typename T>
class IpcQueue {
    public:
        IpcQueue(void): isInitialized(false), entrySize(sizeof(T)), depth(0) {}
        virtual ~IpcQueue(void) {}

        bool createQueue(size_t queueDepth);
        bool deleteQueue(void);

        bool reset(void);
        bool send(T value);
        bool receive(T& value);
        size_t remainNum(void);
        size_t emptyNum(void);
        size_t getDepth(void) { return this->depth; }
    protected:
        QueueHandle_t queueHandle;
        bool isInitialized;
        size_t entrySize;
        size_t depth;

};

#endif /* IPCQUEUE_H */
#ifndef TASKBASE_H
#define TASKBASE_H

#include <stdint.h>

#include <Seeed_Arduino_FreeRTOS.h>

class TaskBase {
    public:
        TaskBase(void): isRunning(false) {}
        virtual ~TaskBase(void) { this->deleteTask(); }

        void createTask(size_t stackSize, uint32_t priority);
        void deleteTask(void);
        TaskHandle_t getTaskHandle(void);
        virtual const char* getName(void) = 0;
    protected:
        TaskHandle_t taskHandle;
        bool isRunning;
        void taskMain(void);
        virtual void setup(void) = 0;
        virtual void loop(void) = 0;
};

#endif /* TASKBASE_H */
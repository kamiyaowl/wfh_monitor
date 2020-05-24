#ifndef TASKBASE_H
#define TASKBASE_H

#include <stdint.h>

#include <Seeed_Arduino_FreeRTOS.h>

class TaskBase {
    public:
        TaskBase(void): isRunning(false) {}
        virtual ~TaskBase() { this->deleteTask(); }

        virtual const char* getName(void);
        void createTask(size_t stackSize, uint32_t priority);
        void deleteTask(void);
    protected:
        TaskHandle_t taskHandle;
        bool isRunning;
        void taskMain(void);
        virtual void setup(void);
        virtual void loop(void);
};

#endif /* TASKBASE_H */
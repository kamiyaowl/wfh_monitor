#ifndef TASKBASE_H
#define TASKBASE_H

#include <stdint.h>

#include <Seeed_Arduino_FreeRTOS.h>

template<typename T>
class TaskBase {
    public:
        TaskBase(void);
        virtual ~TaskBase();
        virtual const char* getName(void);
        void createTask(size_t stackSize, uint32_t priority, T param);
        void deleteTask(void);
    protected:
        T param;
        TaskHandle_t taskHandle;
        bool isRunning;
        void run(void);
        virtual void setup(void);
        virtual void loop(void);
};

#endif /* TASKBASE_H */
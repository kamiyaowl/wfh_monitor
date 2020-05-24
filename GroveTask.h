#ifndef GROVETASK_H
#define GROVETASK_H

#include "TaskBase.h"

class GroveTask : public TaskBase {
    public:
        GroveTask(Serial_& serial): serial(serial) {}
        virtual ~GroveTask(void) {}
    private:
        Serial_& serial; /**< for debug */
        void setup(void) override;
        void loop(void) override;
};

#endif /* GROVETASK_H */
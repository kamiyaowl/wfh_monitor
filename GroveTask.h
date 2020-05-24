#ifndef GROVETASK_H
#define GROVETASK_H

#include "TaskBase.h"

/**
 * @brief Grove端子に接続されたIICセンサの値を収集するTaskです
 */
class GroveTask : public TaskBase {
    public:
        GroveTask(Serial_& serial): serial(serial) {}
        virtual ~GroveTask(void) {}
        const char* getName(void) override { return "GroveTask"; }
    private:
        Serial_& serial; /**< for debug */
        void setup(void) override;
        bool loop(void) override;
};

#endif /* GROVETASK_H */
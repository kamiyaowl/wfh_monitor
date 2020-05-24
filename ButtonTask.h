#ifndef BUTTONTASK_H
#define BUTTONTASK_H

#include "TaskBase.h"

/**
 * @brief Grove端子に接続されたIICセンサの値を収集するTaskです
 */
class ButtonTask : public TaskBase {
    public:
        ButtonTask(Serial_& serial): serial(serial) {}
        virtual ~ButtonTask(void) {}
        const char* getName(void) override { return "ButtonTask"; }
    private:
        Serial_& serial; /**< for debug */
        void setup(void) override;
        bool loop(void) override;
};

#endif /* BUTTONTASK_H */
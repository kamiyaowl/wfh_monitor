#ifndef BUTTONTASK_H
#define BUTTONTASK_H

#include "TaskBase.h"

/**
 * @brief Wio Terminalについている上部ボタンと4方向ボタンの値を取得するタスクです
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
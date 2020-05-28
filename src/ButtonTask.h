#ifndef BUTTONTASK_H
#define BUTTONTASK_H

#include "IpcQueueDefs.h"
#include "IpcQueue.h"

#include "TaskBase.h"

/**
 * @brief Wio Terminalについている上部ボタンと4方向ボタンの値を取得するタスクです
 */
class ButtonTask : public TaskBase {
    public:
        ButtonTask(IpcQueue<ButtonStateBmp_t>& sendQueue, Serial_& serial): sendQueue(sendQueue), serial(serial) {}
        virtual ~ButtonTask(void) {}
        const char* getName(void) override { return "ButtonTask"; }
    private:
        IpcQueue<ButtonStateBmp_t>& sendQueue; /**< ボタン入力送信用 */
        uint32_t oldDebounce; /**< 前回のdebounce済の値 */

        Serial_& serial; /**< for debug */
        void setup(void) override;
        bool loop(void) override;
};

#endif /* BUTTONTASK_H */
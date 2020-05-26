#ifndef UITASK_H
#define UITASK_H

#include <LovyanGFX.hpp>

#include "IpcQueueDefs.h"
#include "IpcQueue.h"

#include "TaskBase.h"

/**
 * @brief UserInterfaceの表示を行うタスクです
 */
class UiTask : public TaskBase {
    public:
        UiTask(IpcQueue<MeasureData_t>& recvMeasureDataQueue,
               IpcQueue<ButtonStateBmp_t>& recvButtonStateQueue,
               Serial_& serial,
               LGFX& lcd,
               LGFX_Sprite& sprite): counter(0), recvMeasureDataQueue(recvMeasureDataQueue), recvButtonStateQueue(recvButtonStateQueue), serial(serial), lcd(lcd), sprite(sprite) {}
         virtual ~UiTask(void) {}
        const char* getName(void) override { return "UiTask"; }
    private:
        IpcQueue<MeasureData_t>& recvMeasureDataQueue; /**< 測定データ受信用 */
        IpcQueue<ButtonStateBmp_t>& recvButtonStateQueue; /**< ボタン入力受信用 */

        MeasureData_t latestMeasureData;
        ButtonStateBmp_t latestButtonState;

        uint32_t counter; /**< for debug*/
        Serial_& serial; /**< for debug */
        LGFX& lcd;
        LGFX_Sprite& sprite;
        void setup(void) override;
        bool loop(void) override;
};

#endif /* UITASK_H */
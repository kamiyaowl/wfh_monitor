#ifndef UITASK_H
#define UITASK_H

#include <LovyanGFX.hpp>

#include "IpcQueueDefs.h"
#include "IpcQueue.h"

#include "ui/BrightnessControl.h"

#include "FpsControlTask.h"

// BrightnessControlでの設定ポイント数
static constexpr size_t UiTaskBrightnessPoint = 4;

/**
 * @brief UserInterfaceの表示を行うタスクです
 */
class UiTask : public FpsControlTask {
    public:
        /**
         * @brief Construct a new Ui Task object
         * 
         * @param recvMeasureDataQueue 測定データの受信Queue
         * @param recvButtonStateQueue ボタン入力の受信Queue
         * @param serial UARTペリフェラル
         * @param lcd LCD Library、事前にinitは済ませておくこと(Wio Terminalに付随しているため)
         * @param sprite 
         */
        UiTask(IpcQueue<MeasureData>& recvMeasureDataQueue,
               IpcQueue<ButtonEventData>& recvButtonStateQueue,
               Serial_& serial,
               LGFX& lcd,
               LGFX_Sprite& sprite): counter(0), recvMeasureDataQueue(recvMeasureDataQueue), recvButtonStateQueue(recvButtonStateQueue), serial(serial), lcd(lcd), sprite(sprite), brightness(lcd) {}

        /**
        * @brief Destroy the Ui Task object
        */
         virtual ~UiTask(void) {}
        const char* getName(void) override { return "UiTask"; }
    protected:
        IpcQueue<MeasureData>& recvMeasureDataQueue; /**< 測定データ受信用 */
        IpcQueue<ButtonEventData>& recvButtonStateQueue; /**< ボタン入力受信用 */

        MeasureData latestMeasureData;
        ButtonEventData latestButtonState;

        uint32_t counter; /**< for debug*/
        Serial_& serial; /**< for debug */
        LGFX& lcd;
        LGFX_Sprite& sprite;
        BrightnessControl<UiTaskBrightnessPoint, LGFX> brightness;

        void setup(void) override;
        bool loop(void) override;
};

#endif /* UITASK_H */
#ifndef UITASK_H
#define UITASK_H

#include <cfloat>
#include <LovyanGFX.hpp>

#include "SysTimer.h"

#include "SharedResourceDefs.h"
#include "IpcQueueDefs.h"
#include "IpcQueue.h"

#include "ui/BrightnessControl.h"

#include "FpsControlTask.h"

/**
 * @brief UserInterfaceの表示を行うタスクです
 * @tparam N BrightnessControlの設定KeyPoint数
 */
template<int N>
class UiTask : public FpsControlTask {
    public:
        /**
         * @brief Construct a new Ui Task object
         * 
         * @param resource 共有リソース群
         * @param recvMeasureDataQueue 測定データの受信Queue
         * @param recvButtonStateQueue ボタン入力の受信Queue
         * @param lcd LCD Library、事前にinitは済ませておくこと(Wio Terminalに付随しているため)
         * @param sprite 
         */
        UiTask(
            const SharedResourceDefs& resource,
            IpcQueue<MeasureData>& recvMeasureDataQueue,
            IpcQueue<ButtonEventData>& recvButtonStateQueue,
            LGFX& lcd,
            LGFX_Sprite& sprite
        ): resource(resource), counter(0), recvMeasureDataQueue(recvMeasureDataQueue), recvButtonStateQueue(recvButtonStateQueue), lcd(lcd), sprite(sprite), brightness(lcd) {}

        /**
        * @brief Destroy the Ui Task object
        */
         virtual ~UiTask(void) {}
        const char* getName(void) override { return "UiTask"; }
    protected:
        const SharedResourceDefs& resource; /**< 共有リソース群 */
        IpcQueue<MeasureData>& recvMeasureDataQueue; /**< 測定データ受信用 */
        IpcQueue<ButtonEventData>& recvButtonStateQueue; /**< ボタン入力受信用 */

        MeasureData latestMeasureData;
        ButtonEventData latestButtonState;

        uint32_t counter; /**< for debug*/
        LGFX& lcd;
        LGFX_Sprite& sprite;
        BrightnessControl<N, LGFX> brightness;

        void setup(void) override {
            // fps control
            this->setFps(30);

            // initialize lcd
            this->lcd.setTextSize(1);

            // initial value
            this->latestMeasureData.visibleLux = 0.0f;
            this->latestMeasureData.tempature = 0.0f;
            this->latestMeasureData.pressure = 0.0f;
            this->latestMeasureData.humidity = 0.0f;
            this->latestMeasureData.gas = 0.0f;
            this->latestMeasureData.timestamp = 0x0;
            this->latestButtonState.raw = 0x0;
            this->latestButtonState.debounce = 0x0;
            this->latestButtonState.push = 0x0;
            this->latestButtonState.release = 0x0;

            // initialize submodules
            static constexpr BrightnessSetting brightnessSetting[N] = { // TODO: 設定ファイルから色々できるといいなぁ...
                { .visibleLux =  50.0f , .brightness = 20 },
                { .visibleLux = 120.0f , .brightness = 100 },
                { .visibleLux = 180.0f , .brightness = 200 },
                { .visibleLux = FLT_MAX, .brightness = 255 },
            };
            this->brightness.configure(true, 4000, 2000, brightnessSetting);
        }
        bool loop(void) override {
            // receive queue datas
            if (this->recvMeasureDataQueue.remainNum() > 0) {
                this->recvMeasureDataQueue.receive(&this->latestMeasureData, false);
            }
            if (this->recvButtonStateQueue.remainNum() > 0) {
                this->recvButtonStateQueue.receive(&this->latestButtonState, false);
            }

            // backlight
            brightness.update(this->latestMeasureData.visibleLux);

            // test
            this->lcd.setCursor(0,0);
            this->lcd.printf("#UiTask\n");
            this->lcd.printf("systick = %d\n", SysTimer::getTickCount());
            this->lcd.printf("counter = %d\n", this->counter);
            this->lcd.printf("maxFps  = %f\n", this->getFpsWithoutDelay());
            this->lcd.printf("\n");

            this->lcd.printf("#SensorData\n");
            this->lcd.printf("visibleLux = %f\n", this->latestMeasureData.visibleLux);
            this->lcd.printf("tempature  = %f\n", this->latestMeasureData.tempature);
            this->lcd.printf("pressure   = %f\n", this->latestMeasureData.pressure);
            this->lcd.printf("humidity   = %f\n", this->latestMeasureData.humidity);
            this->lcd.printf("gas        = %f\n", this->latestMeasureData.gas);
            this->lcd.printf("timestamp  = %u\n", this->latestMeasureData.timestamp);
            this->lcd.printf("\n");

            this->lcd.printf("#Button\n");
            this->lcd.printf("raw       = %08x\n", this->latestButtonState.raw);
            this->lcd.printf("debounce  = %08x\n", this->latestButtonState.debounce);
            this->lcd.printf("push      = %08x\n", this->latestButtonState.push);
            this->lcd.printf("release   = %08x\n", this->latestButtonState.release);
            this->lcd.printf("timestamp = %u\n", this->latestButtonState.timestamp);
            this->lcd.printf("\n");
            
            // for debug
            this->counter++;

            return false; /**< no abort */
        }
};

#endif /* UITASK_H */
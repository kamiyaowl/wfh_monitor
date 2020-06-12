#ifndef UITASK_H
#define UITASK_H

#include <cfloat>
#include <LovyanGFX.hpp>

#include "../SharedResourceDefs.h"
#include "../IpcQueueDefs.h"
#include "../IpcQueue.h"
#include "../SysTimer.h"
#include "../FpsControlTask.h"

#include "control/BrightnessControl.h"

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
         * @param sendWifiReqQueue Wifi関係の要求Queue
         * @param recvWifiRespQueue Wifi関係の応答Queue
         * @param lcd LCD Library、事前にinitは済ませておくこと(Wio Terminalに付随しているため)
         * @param sprite 
         */
        UiTask(
            const SharedResourceDefs& resource,
            IpcQueue<MeasureData>& recvMeasureDataQueue,
            IpcQueue<ButtonEventData>& recvButtonStateQueue,
            IpcQueue<WifiTaskRequest>& sendWifiReqQueue,
            IpcQueue<WifiTaskResponse>& recvWifiRespQueue,
            LGFX& lcd,
            LGFX_Sprite& sprite
        ): resource(resource),           
           recvMeasureDataQueue(recvMeasureDataQueue),
           recvButtonStateQueue(recvButtonStateQueue),
           sendWifiReqQueue(sendWifiReqQueue),
           recvWifiRespQueue(recvWifiRespQueue),
           lcd(lcd),
           sprite(sprite),
           brightness(lcd) {}

        /**
        * @brief Destroy the Ui Task object
        */
         virtual ~UiTask(void) {}
        const char* getName(void) override { return "UiTask"; }
    protected:
        const SharedResourceDefs& resource; /**< 共有リソース群 */
        IpcQueue<MeasureData>& recvMeasureDataQueue; /**< 測定データ受信用 */
        IpcQueue<ButtonEventData>& recvButtonStateQueue; /**< ボタン入力受信用 */
        IpcQueue<WifiTaskRequest>& sendWifiReqQueue; /**< Wifi要求 */
        IpcQueue<WifiTaskResponse>& recvWifiRespQueue; /**< Wifi応答  */
        // hw
        LGFX& lcd;
        LGFX_Sprite& sprite;
        // hw resourceを使って初期化が必要
        BrightnessControl<N, LGFX> brightness;
        // configから読み出し
        uint32_t ambientIntervalMs; /**< ambientへのデータ送信周期 */
        bool isUseAmbient; /**< ambientへのデータ送信を利用する場合はtrue */
        // ローカル変数
        bool isSendingAmbient; /**< ambientへデータ送信中の場合はtrue, QD=1制御用フラグ */
        bool wasSucceedSendAmbient; /**< 最後にAmbientにデータ送信した結果 */
        uint32_t counter; /**< for debug*/
        MeasureData latestMeasureData; /**< 最後に受信した測定データ */
        ButtonEventData latestButtonState; /**< 最後に受信したボタン入力 */
        WifiStatusData latestWifiStatus; /**< 最後に受信したWiFi Status */

        void setup(void) override {
            // initialize lcd
            this->lcd.setTextSize(1);

            // configure
            static constexpr BrightnessSetting brightnessSetting[N] = { // TODO: 設定ファイルから色々できるといいなぁ...
                { .visibleLux =  50.0f , .brightness = 20 },
                { .visibleLux = 120.0f , .brightness = 100 },
                { .visibleLux = 180.0f , .brightness = 200 },
                { .visibleLux = FLT_MAX, .brightness = 255 },
            };

            this->resource.config.operate([&](GlobalConfig<FixedConfig::ConfigAllocateSize>& config){
                // fps
                auto fps = GlobalConfigDefaultValues::UiTaskFps;
                config.read(GlobalConfigKeys::UiTaskFps, fps);
                this->setFps(fps);
                // auto brightness
                auto holdMs = GlobalConfigDefaultValues::BrightnessHoldMs;
                auto transitionMs = GlobalConfigDefaultValues::BrightnessTransitionMs;
                config.read(GlobalConfigKeys::BrightnessHoldMs, holdMs);
                config.read(GlobalConfigKeys::BrightnessTransitionMs, transitionMs);
                this->brightness.configure(true, holdMs, transitionMs, brightnessSetting);
                // ambient
                this->isUseAmbient = GlobalConfigDefaultValues::UseAmbient;
                this->ambientIntervalMs = GlobalConfigDefaultValues::AmbientIntervalMs;
                config.read(GlobalConfigKeys::UseAmbient, this->isUseAmbient);
                config.read(GlobalConfigKeys::AmbientIntervalMs, this->ambientIntervalMs);
            });

            // initial value
            this->isSendingAmbient = false;
            this->wasSucceedSendAmbient = false;
            this->counter = 0x0;
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
            this->latestWifiStatus.ipAddr[0] = 0x0;
            this->latestWifiStatus.ipAddr[1] = 0x0;
            this->latestWifiStatus.ipAddr[2] = 0x0;
            this->latestWifiStatus.ipAddr[3] = 0x0;
            this->latestWifiStatus.status = WL_DISCONNECTED;
            this->latestWifiStatus.timestamp = 0x0;
        }
        bool loop(void) override {
            // receive queue datas
            if (this->recvMeasureDataQueue.remainNum() > 0) {
                this->recvMeasureDataQueue.receive(&this->latestMeasureData, false);
            }
            if (this->recvButtonStateQueue.remainNum() > 0) {
                this->recvButtonStateQueue.receive(&this->latestButtonState, false);
            }
            if (this->recvWifiRespQueue.remainNum() > 0) {
                WifiTaskResponse resp;
                this->recvWifiRespQueue.receive(&resp, false);
                switch (resp.id) {
                    case WifiTaskRequestId::Nop:
                        break;
                    case WifiTaskRequestId::GetWifiStatus:
                        this->latestWifiStatus = resp.data.wifiStatus;
                        break;
                    case WifiTaskRequestId::SendSensorData:
                        this->isSendingAmbient = false;
                        this->wasSucceedSendAmbient = resp.isSuccess;
                        break;
                    default:
                        // ありえん
                        break;
                }
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
            this->lcd.printf("timestamp = %u\n"  , this->latestButtonState.timestamp);
            this->lcd.printf("\n");

            this->lcd.printf("#Wifi\n");
            this->lcd.printf("status    = %d\n"         , this->latestWifiStatus.status);
            this->lcd.printf("ipAddr    = %d.%d.%d.%d\n", this->latestWifiStatus.ipAddr[0], this->latestWifiStatus.ipAddr[1], this->latestWifiStatus.ipAddr[2], this->latestWifiStatus.ipAddr[3]);
            this->lcd.printf("timestamp = %u\n"         , this->latestWifiStatus.timestamp);
            this->lcd.printf("\n");

            this->lcd.printf("#Ambient\n");
            this->lcd.printf("use     = %d\n"         , this->isUseAmbient);
            this->lcd.printf("sending = %d\n"         , this->isSendingAmbient);
            this->lcd.printf("result  = %d\n"         , this->wasSucceedSendAmbient);
            this->lcd.printf("\n");

            // TODO: #54 常にStatus監視しないようにする(定期実行としたい)
            if (this->sendWifiReqQueue.remainNum() == 0) {
                WifiTaskRequest req;
                req.id = WifiTaskRequestId::GetWifiStatus;
                this->sendWifiReqQueue.send(&req);
                // TODO: Test Codeなので削除, 置き換えるときはQueue Fullに注意
                if (this->isUseAmbient && !this->isSendingAmbient) {
                    this->isSendingAmbient = true; // QD=1制限用

                    req.id = WifiTaskRequestId::SendSensorData;
                    req.data.measureData = this->latestMeasureData;
                    this->sendWifiReqQueue.send(&req);
                }
            }

            // for debug
            this->counter++;

            return false; /**< no abort */
        }
};

#endif /* UITASK_H */
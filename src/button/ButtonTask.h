#ifndef BUTTONTASK_H
#define BUTTONTASK_H

#include "../SharedResourceDefs.h"
#include "../IpcQueueDefs.h"
#include "../IpcQueue.h"
#include "../SysTimer.h"
#include "../FpsControlTask.h"

/**
 * @brief Wio Terminalについている上部ボタンと4方向ボタンの値を取得するタスクです
 * 
 * @tparam N debounceする履歴値数
 */
template<size_t N>
class ButtonTask : public FpsControlTask {
    public:

        /**
         * @brief Construct a new Button Task object
         * 
         * @param resource 共有リソース群
         * @param sendQueue ボタン入力の送信Queue
         */
        ButtonTask(
            const SharedResourceDefs& resource,
            IpcQueue<ButtonEventData>& sendQueue
        ): resource(resource), sendQueue(sendQueue) {}

        /**
         * @brief Destroy the Button Task object
         */
        virtual ~ButtonTask(void) {}
        const char* getName(void) override { return "ButtonTask"; }
    protected:
        const SharedResourceDefs& resource; /**< 共有リソース群 */
        IpcQueue<ButtonEventData>& sendQueue; /**< ボタン入力送信用 */
        uint32_t recents[N];  /**< debounce用の履歴値 */
        uint32_t recentsPtr;  /**< 次に書き込むrecentsのindex */
        uint32_t oldDebounce; /**< 前回のdebounce済の値 */

        void setup(void) override {
            // configure
            this->resource.config.operate([&](GlobalConfig<FixedConfig::ConfigAllocateSize>& config){
                // fps
                auto fps = GlobalConfigDefaultValues::ButtonTaskFps;
                config.read(GlobalConfigKeys::ButtonTaskFps, fps);
                this->setFps(fps);
            });

            // port initialize
            pinMode(WIO_5S_UP,    INPUT_PULLUP);
            pinMode(WIO_5S_DOWN,  INPUT_PULLUP);
            pinMode(WIO_5S_LEFT,  INPUT_PULLUP);
            pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
            pinMode(WIO_5S_PRESS, INPUT_PULLUP);
            pinMode(WIO_KEY_A,    INPUT_PULLUP);
            pinMode(WIO_KEY_B,    INPUT_PULLUP);
            pinMode(WIO_KEY_C,    INPUT_PULLUP);

            // variable initialize
            this->oldDebounce = static_cast<uint32_t>(ButtonState::None);
            this->recentsPtr  = 0;
            for (uint32_t i = 0; i < N; i++) {
                this->recents[i] = 0;
            }
        }

        bool loop(void) override {
            // Queueに空きがない場合、edgeが抜け落ちる可能性があるので何もしない
            if (this->sendQueue.emptyNum() == 0) {
                return false; /**< no abort */            
            }

            // get raw button input
            uint32_t raw = 0x0;
            raw |= (digitalRead(WIO_5S_UP)    == LOW) ? static_cast<uint32_t>(ButtonState::Up)    : static_cast<uint32_t>(ButtonState::None);
            raw |= (digitalRead(WIO_5S_DOWN)  == LOW) ? static_cast<uint32_t>(ButtonState::Down)  : static_cast<uint32_t>(ButtonState::None);
            raw |= (digitalRead(WIO_5S_LEFT)  == LOW) ? static_cast<uint32_t>(ButtonState::Left)  : static_cast<uint32_t>(ButtonState::None);
            raw |= (digitalRead(WIO_5S_RIGHT) == LOW) ? static_cast<uint32_t>(ButtonState::Right) : static_cast<uint32_t>(ButtonState::None);
            raw |= (digitalRead(WIO_5S_PRESS) == LOW) ? static_cast<uint32_t>(ButtonState::Press) : static_cast<uint32_t>(ButtonState::None);
            raw |= (digitalRead(WIO_KEY_A)    == LOW) ? static_cast<uint32_t>(ButtonState::A)     : static_cast<uint32_t>(ButtonState::None);
            raw |= (digitalRead(WIO_KEY_B)    == LOW) ? static_cast<uint32_t>(ButtonState::B)     : static_cast<uint32_t>(ButtonState::None);
            raw |= (digitalRead(WIO_KEY_C)    == LOW) ? static_cast<uint32_t>(ButtonState::C)     : static_cast<uint32_t>(ButtonState::None);

            // add recents
            this->recents[this->recentsPtr] = raw;
            this->recentsPtr = (this->recentsPtr + 1) % N;

            // calc debounced value
            uint32_t currentDebounce = UINT32_MAX;
            for (uint32_t i = 0; i < N; i++) {
                currentDebounce &= this->recents[i];
            }

            // edge detect
            const uint32_t push    = currentDebounce   & (currentDebounce ^ this->oldDebounce); // 差分かつ今回いるもの
            const uint32_t release = this->oldDebounce & (currentDebounce ^ this->oldDebounce); // 差分かつ前回いるもの

            // send Queue
            const ButtonEventData data = {
                .raw = raw,
                .debounce = currentDebounce,
                .push = push,
                .release = release,
                .timestamp = SysTimer::getTickCount(),
            };
            this->sendQueue.send(&data);

            // update oldDebounce
            this->oldDebounce = currentDebounce;

            return false; /**< no abort */            
        }
};

#endif /* BUTTONTASK_H */
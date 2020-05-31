#ifndef BRIGHTNESSCONTROL_H
#define BRIGHTNESSCONTROL_H

// for serial print debug
#ifdef WFH_MONITOR_ENABLE_SERIAL_PRINT_BRIGHTNESS_CONTROL
#define SERIAL_PRINT_BRIGHTNESS_CONTROL(...) (Serial.printf(__VA_ARGS__))
#else
#define SERIAL_PRINT_BRIGHTNESS_CONTROL(...)
#endif

#include <cstdint>

#include "../SysTimer.h" //TODO: できれば相対インクルードやめたい

/**
 * @brief BrightnessControlの遷移状態を示します
 */
enum class BrightnessControlState : uint32_t {
    Disable, /**< 自動調光無効化 */
    Enable, /**< 自動調光有効化後 */
    Watch, /**< 自動調光有効化中、該当するsetttingを見つけて一定時間立つことを待っている途中 */
    Transition, /**< 明るさ遷移中 */
};

/**
 * @brief 調光設定定義を示します
 */
struct BrightnessSetting {
    float visibleLux; /**< 光センサの値[lux] */
    uint8_t brightness; /**< visibleLuxが上記以下のときに設定する値(0-255) */
};

/**
 * @brief バックライトの自動調光を行うクラスです
 * 
 * @tparam N 調光設定のポイント数。最低2Pointの設定が必要です
 * @tparam TLCD setBrightnessが使えるLCD Library
 */
template<size_t N, typename TLCD>
class BrightnessControl {
    public:
        /**
         * @brief Construct a new Brightness Control object
         * 
         * @param lcd LCD制御ライブラリ、setBrightnessを使用する
         */
        BrightnessControl(TLCD& lcd): lcd(lcd) {
            this->clear();
        }
        virtual ~BrightnessControl(void) {}

        /**
         * @brief 設定をクリアし、自動調光を無効化します
         */
        void clear(void) {
            this->state = BrightnessControlState::Disable;
            this->avaibleSettingCount = 0;
            for (uint32_t i = 0; i < N; i++) {
                this->settings[i] = {
                    .visibleLux = 0.0f,
                    .brightness = 0,
                };
            }
        }

        /**
         * @brief 超高設定を反映します
         * 
         * @param isEnable 設定反映後自動調光を有効にする場合はtrue
         * @param holdMs 周囲の明るさが変動しなくなったと判定する期間[ms]
         * @param transitionMs 遷移するのに必要な期間[ms]
         * @param settings 調光設定、visibleLuxが昇順ではない場合は無視されます
         * 
         * @note N=4だが2ポイントで良い場合、0,1に設定を入れて2にvisibleLux=0の値を入れることで2ポイント制御できます
         */
        void configure(bool isEnable, uint32_t holdMs, uint32_t transitionMs, const BrightnessSetting settings[N]) {
            this->avaibleSettingCount = 0;
            for (uint32_t i = 0; i < N; i++) {
                BrightnessSetting s = settings[i];
                // 前回のvisibleLuxより値が大きくなければ設定終了
                if (i > 0 && (settings[i-1].visibleLux >= s.visibleLux)) {
                    break;
                }
                // 設定を反映
                this->settings[i] = s;
                this->avaibleSettingCount++;
            }
            // その他の設定
            this->holdTick = SysTimer::msToTick(holdMs);
            this->transitionTick = SysTimer::msToTick(transitionMs);
            // 調光有効化, 有効な設定がない場合は有効化されない
            this->setEnable(isEnable);
            if (this->state == BrightnessControlState::Enable) {
                // とりあえずsetting=0を反映しておく
                this->currentIndex = 0;
                this->lcd.setBrightness(this->settings[0].brightness);
            }
        }

        /**
         * @brief 自動調光の切り替えを行います
         * @note configureが正常に完了していない場合、Disableの状態で保持されます
         * 
         * @param isEnable 自動で行う場合はtrue
         */
        void setEnable(bool isEnable) {
            if (this->avaibleSettingCount > 0) {
                this->state = isEnable ? BrightnessControlState::Enable : BrightnessControlState::Disable;
                this->watchIndex = 0;
                this->watchStartTick = SysTimer::getTickCount();
            }
        }

        /**
         * @brief 指定した値をそのまま輝度に反映します。この関数を使うと強制的にState=Disableに遷移します
         * @note 再度自動調光する場合はsetEnable(true)を呼び出します
         * 
         * @param brightness 設定する値0-255
         */
        void setManual(uint8_t brightness) {
            this->lcd.setBrightness(brightness);
            this->state = BrightnessControlState::Disable;
        }

        /**
         * @brief 最新のbrighenessを更新し、必要に応じてバックライトの設定を更新します
         * 
         * @param visibleLux 照度センサの値[lux]
         * @retval bool Transitionに遷移した場合はtrue
         */
        BrightnessControlState update(float visibleLux) {
            SERIAL_PRINT_BRIGHTNESS_CONTROL("update(visibleLux=%f) state=%d, currentIndex=%d\n", visibleLux, this->state, this->currentIndex);
            switch (this->state) {
                case BrightnessControlState::Disable:
                    // 設定無効, マニュアル設定後は何もしない
                    break;
                case BrightnessControlState::Enable: {
                    // 該当するindexを算出する
                    uint32_t dstIndex = 0;
                    if (!this->searchSetting(visibleLux, dstIndex)) {
                        // 有効な設定を見つけられなかった
                        SERIAL_PRINT_BRIGHTNESS_CONTROL("[Enable->Enable] not found\n");
                        break;
                    }
                    // 現在の設定と異なっていればWatch開始
                    if (dstIndex != this->currentIndex) {
                        this->watchIndex = dstIndex;
                        this->watchStartTick = SysTimer::getTickCount();
                        this->state = BrightnessControlState::Watch;
                        SERIAL_PRINT_BRIGHTNESS_CONTROL("[Enable->Watch] idx=%d, startTick=%d\n", this->watchIndex, this->watchStartTick);
                    }
                    break;
                }
                case BrightnessControlState::Watch: {
                    // 該当するindexを算出する
                    uint32_t dstIndex = 0;
                    if (!this->searchSetting(visibleLux, dstIndex)) {
                        // 有効な設定を見つけられなかった->最初に戻る
                        this->state = BrightnessControlState::Enable;
                        SERIAL_PRINT_BRIGHTNESS_CONTROL("[Watch->Enable] not found\n");
                        break;
                    }
                    // Watch中のIndexと一致しなくなったら最初に戻る
                    if (dstIndex != this->watchIndex) {
                        this->state = BrightnessControlState::Enable;
                        SERIAL_PRINT_BRIGHTNESS_CONTROL("[Watch->Enable] mismatch dstIndex=%d, wathcIndex=%d\n", dstIndex, this->watchIndex);
                        break;
                    }
                    // 規定時間WatchできたらTransition開始
                    const uint32_t currentTick = SysTimer::getTickCount();
                    const uint32_t diff = SysTimer::diff(this->watchStartTick, currentTick);
                    if (diff > this->holdTick) {
                        this->transitionStartTick = currentTick;
                        this->transitionSrcIndex = this->currentIndex;
                        this->transitionDstIndex = this->watchIndex;
                        this->state = BrightnessControlState::Transition;
                        SERIAL_PRINT_BRIGHTNESS_CONTROL("[Watch->Transition] startTick=%d srcIndex=%d, dstIndex=%d\n", this->watchStartTick, this->transitionSrcIndex, this->transitionDstIndex);
                    }
                    break;
                }
                case BrightnessControlState::Transition: {
                    // 遷移状態の割合を算出
                    const uint32_t currentTick = SysTimer::getTickCount();
                    const uint32_t diffTick = SysTimer::diff(this->transitionStartTick, currentTick);
                    // 設定時間立っていれば最終値を設定して終了
                    if (diffTick > this->transitionTick) {
                        // 最終値を設定
                        this->lcd.setBrightness(this->settings[this->transitionDstIndex].brightness);
                        SERIAL_PRINT_BRIGHTNESS_CONTROL("[Transition] setBrightness(%d)\n", this->settings[this->transitionDstIndex].brightness);
                        // ステータスを更新してEnableに戻る
                        this->currentIndex = this->transitionDstIndex;
                        this->state = BrightnessControlState::Enable;
                        SERIAL_PRINT_BRIGHTNESS_CONTROL("[Transition->Enable] startTick=%d srcIndex=%d, dstIndex=%d\n", this->watchStartTick, this->transitionSrcIndex, this->transitionDstIndex);
                        break;
                    }
                    // 線形補間で設定値を決める
                    const float rate = diffTick / static_cast<float>(this->transitionTick); // 事前にdiff <= transitionTickは確認済
                    const float src  = static_cast<float>(this->settings[this->transitionSrcIndex].brightness); // 負数の扱いが面倒なので先にfloatにする
                    const float dst  = static_cast<float>(this->settings[this->transitionDstIndex].brightness);
                    const uint8_t brightness = static_cast<uint8_t>((dst - src) * rate + src);
                    // 反映
                    this->lcd.setBrightness(brightness);
                    SERIAL_PRINT_BRIGHTNESS_CONTROL("[Transition] setBrightness(%d)\n", brightness);
                    break;
                }
            }
            // 現在のstateを返す
            return this->state;
        }

        /**
         * @brief Get the State
         * 
         * @return BrightnessControlState 現在の状態
         */
        BrightnessControlState getState(void) {
            return this->state;
        }

    protected:
        /**
         * @brief 該当する設定Indexを返します
         * 
         * @param visibleLux 照度センサの値[lux]
         * @param index 見つけたindexが代入されます
         * @return false 設定が存在しない、有効な設定内に合致する値ではなかった
         */
        bool searchSetting(float visibleLux, uint32_t& index) {
            // 設定が存在しない
            if (this->avaibleSettingCount == 0) {
                return false;
            }
            // 有効な設定を順に探して、エッジを見つける
            for (uint32_t i = 0; i < this->avaibleSettingCount - 1; i++) {
                const bool isMatchCurrent = (visibleLux < this->settings[i].visibleLux);
                const bool isMatchNext    = (visibleLux < this->settings[i + 1].visibleLux);
                // 現在の内容にマッチして、次の内容にマッチしない(もしくは存在しない)indexが境界
                if (isMatchCurrent && isMatchNext) {
                    index = i; // 見つけたindexをセット
                    SERIAL_PRINT_BRIGHTNESS_CONTROL("searchSetting(visibleLux=%f) ret true. index=%d\n", visibleLux, index);
                    return true;
                }
            }
            // 一番最後の要素のみ検査
            const bool isMatchLast = (visibleLux < this->settings[this->avaibleSettingCount - 1].visibleLux);
            if (isMatchLast) {
                index = this->avaibleSettingCount - 1; // 最後の要素
                SERIAL_PRINT_BRIGHTNESS_CONTROL("searchSetting(visibleLux=%f) ret true. lastIndex=%d\n", visibleLux, index);
                return true;
            }
            // どれにも合致しなかった
            SERIAL_PRINT_BRIGHTNESS_CONTROL("searchSetting(visibleLux=%f) ret false\n", visibleLux);
            return false;
        }

        // base
        TLCD& lcd;
        BrightnessControlState state; /**< 現在のステータス */
        uint32_t currentIndex; /**< 現在反映しているsetting */
        // configuration
        BrightnessSetting settings[N]; /**< brightnessに対応する 設定値、昇順である必要がある */
        size_t avaibleSettingCount; /**< settingsに正常にセットされているポイント数 */
        uint32_t holdTick; /**< 条件を満たしてTransisionを有効にする期間 */
        uint32_t transitionTick; /**< 遷移する時間 */
        // enable->watch
        uint32_t watchStartTick; /**< 最後にマッチした設定を確認した時刻 */
        size_t watchIndex; /**< 最後に一致した該当する設定 */
        // watch->transition
        uint32_t transitionStartTick; /**< 遷移開始時刻 */
        uint32_t transitionSrcIndex; /**< 遷移元設定 */
        uint32_t transitionDstIndex; /**< 遷移先設定 */
};

#endif /* BRIGHTNESSCONTROL_H */
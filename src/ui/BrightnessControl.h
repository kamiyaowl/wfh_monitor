#ifndef BRIGHTNESSCONTROL_H
#define BRIGHTNESSCONTROL_H

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
         * @note 4ポイントで実体化しるものの設定は2ポイントで良い場合、0,1に設定を入れて2,3にはvisibleLux=0の値を入れることで2ポイント制御できます
         */
        void configure(bool isEnable, uint32_t holdMs, uint32_t transitionMs, BrightnessSetting settings[N]) {
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
            switch (this->state) {
                case BrightnessControlState::Disable:
                    // 設定無効, マニュアル設定後は何もしない
                    break;
                case BrightnessControlState::Enable: {
                    // 該当するindexを算出する
                    uint32_t dstIndex = 0;
                    if (!this->searchSetting(visibleLux, dstIndex)) {
                        // 有効な設定を見つけられなかった
                        break;
                    }
                    // 現在の設定と異なっていればWatch開始
                    if (dstIndex != this->currentIndex) {
                        this->watchIndex = dstIndex;
                        this->watchStartTick = SysTimer::getTickCount();
                        this->state = BrightnessControlState::Watch;
                    }
                }
                case BrightnessControlState::Watch: {
                    // 該当するindexを算出する
                    uint32_t dstIndex = 0;
                    if (!this->searchSetting(visibleLux, dstIndex)) {
                        // 有効な設定を見つけられなかった
                        break;
                    }
                    // Watch中のIndexと一致しなくなったら最初に戻る
                    if (dstIndex != this->watchIndex) {
                        this->state = BrightnessControlState::Enable;
                    }
                    // 規定時間WatchできたらTransition開始
                    const uint32_t currentTick = SysTimer::getTickCount();
                    const uint32_t diff = SysTimer::diff(this->watchStartTick, currentTick);
                    if (diff > this->holdTick) {
                        this->startTick = currentTick;
                        this->srcSetting = this->currentIndex;
                        this->dstSetting = this->watchIndex;
                        this->state = BrightnessControlState::Transition;
                    }
                }
                case BrightnessControlState::Transition: {
                    
                }
                default:
                    break;
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
         * @return true 有効な値を見つけた
         * @return false 設定が存在しない、有効な設定内に合致する値ではなかった
         */
        bool searchSetting(float visibleLux, uint32_t& index) {
            // 有効な設定を順に探して、エッジを見つける
            for (uint32_t i = 0; i < this->avaibleSettingCount; i++) {
                const bool isMatchCurrent = (visibleLux < this->settings[i].visibleLux);
                const bool isExistNext    = (i < (this->avaibleSettingCount - 1));
                const bool isMatchNext    = isExistNext && (visibleLux < this->settings[i].visibleLux); // isExistNextを見ないとOutOfIndexする
                // 現在の内容にマッチして、次の内容にマッチしない(もしくは存在しない)indexが境界
                if (isMatchCurrent && !isMatchNext) {
                    index = i; // 見つけたindexをセット
                    return true;
                }
            }
            // なかった場合
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
        uint32_t startTick; /**< 遷移開始時刻 */
        uint32_t srcSetting; /**< 遷移元設定 */
        uint32_t dstSetting; /**< 遷移先設定 */
};

#endif /* BRIGHTNESSCONTROL_H */
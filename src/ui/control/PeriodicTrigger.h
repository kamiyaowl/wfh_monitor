#ifndef PERIODICTRIGGER_H
#define PERIODICTRIGGER_H

#include <cstdint>

#include "../../SysTimer.h"

/**
 * @brief 周期タスク制御を行います
 */
class PeriodicTrigger {
    public:
        /**
         * @brief Construct a new Periodic Trigger object
         */
        PeriodicTrigger(void): durationMs(0), latestTick(0), isEnabled(false) {}

        /**
         * @brief Construct a new Periodic Trigger object
         * @note こちらのコンストラクタは自動的に開始されます
         * 
         * @param durationMs 周期
         */
        PeriodicTrigger(uint32_t durationMs): durationMs(0), latestTick(0), isEnabled(false) {
            this->start(durationMs);
        }

        /**
         * @brief Destroy the Periodic Trigger object
         */
        virtual ~PeriodicTrigger(void) {}

        /**
         * @brief 周期タスクの監視を開始します
         * 
         * @param durationMs 周期
         */
        void start(uint32_t durationMs) {
            this->durationMs = durationMs;
            this->latestTick = SysTimer::getTickCount();
            this->isEnabled = true;
        }

        /**
         * @brief 定期監視を停止します。再開する場合はstart()を呼び出してください。
         */
        void stop(void) {
            this->isEnabled = false;
        }

        /**
         * @brief 毎更新ごと呼び出します
         * 
         * @tparam F any(void)で呼び出せる型
         * @param func 呼び出し先関数
         */
        template<class F>
        void update(F func) {
            // 登録済ではない場合
            if (!this->isEnabled) {
                return;
            }
            // 最後に呼び出した時刻からの差分を取得
            const uint32_t currentTick = SysTimer::getTickCount();
            const uint32_t diffTick    = SysTimer::diff(this->latestTick, currentTick);
            const uint32_t diffMs      = SysTimer::tickToMs(diffTick);
            if (diffMs < this->durationMs) {
                return;
            }
            // 最終時刻を更新して呼び出し
            this->latestTick = currentTick;
            func();
        }
    protected:
        uint32_t durationMs; /**< 設定周期 */
        uint32_t latestTick; /**< 最後に通知した時刻 */
        bool isEnabled; /**< 定期実行が有効ならtrue */

};

#endif /* PERIODICTRIGGER_H */
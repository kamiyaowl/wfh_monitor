#ifndef FPSCONTROLTASK_H
#define FPSCONTROLTASK_H

#include "SysTimer.h"
#include "TaskBase.h"

/**
 * @brief FPS設定可能なタスクの基底クラスです
 */
class FpsControlTask : public TaskBase {
    public:

        /**
         * @brief Construct a new Fps Control Task object
         */
        FpsControlTask(void): durationTick(SysTimer::secToTick(1)), diffTick(0) {}

        /**
         * @brief Destroy the Fps Control Task object
         */
        virtual ~FpsControlTask(void) {}

        /**
         * @brief FPSを再設定します
         * 
         * @param fps 設定したいFPS
         */
        void setFps(uint32_t fps);

        /**
         * @brief loop()での処理時間を取得します
         */
        float getFpsWithoutDelay(void) { return 1.0f / SysTimer::tickToSec<float>(this->diffTick); }

    protected:
        uint32_t durationTick;
        uint32_t diffTick;

        void taskMain(void) override;
};

#endif  /* FPSCONTROLTASK_H */
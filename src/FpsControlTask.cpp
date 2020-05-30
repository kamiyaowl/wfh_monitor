#include "FpsControlTask.h"

void FpsControlTask::setFps(uint32_t fps) {
    // fast
    if (fps == 0) {
        this->durationTick = 0;
        return;
    }
    // Convert: [fps->duration]->tickCount
    const uint32_t durationMs = 1000 / fps;
    this->durationTick = SysTimer::msToTick(durationMs);
}

void FpsControlTask::taskMain(void) {
    bool isAbort = false;

    setup();
    do {
        // 時間計測付きでTaskを実行
        const uint32_t startTick = SysTimer::getTickCount();
        isAbort = loop();
        const uint32_t endTick = SysTimer::getTickCount();

        // 差分を計算
        this->diffTick = SysTimer::diff(startTick, endTick);
        // 時間に余りがあれば一定時間待つ
        if (this->diffTick < this->durationTick) {
            const uint32_t delayTick = this->durationTick - this->diffTick; // 事前に大小関係見てるのでUnderflowしない
            vTaskDelay(delayTick);
        }

    } while(!isAbort);

    // delete itself
    vTaskDelete(NULL);
    this->isRunning = false;
}
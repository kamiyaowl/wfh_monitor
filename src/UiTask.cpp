#include <cfloat>

#include "SysTimer.h"
#include "UiTask.h"

void UiTask::setup(void) {
    // fps control
    this->setFps(30);

    // initialize lcd
    this->lcd.init();
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
    static constexpr BrightnessSetting brightnessSetting[UiTaskBrightnessPoint] = { // TODO: 設定ファイルから色々できるといいなぁ...
        { .visibleLux =  50.0f , .brightness = 20 },
        { .visibleLux = 120.0f , .brightness = 100 },
        { .visibleLux = 180.0f , .brightness = 200 },
        { .visibleLux = FLT_MAX, .brightness = 255 },
    };
    this->brightness.configure(true, 4000, 2000, brightnessSetting);

    // initialize LittlevGL
    // lvgl_setup();
}

bool UiTask::loop(void) {
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

    /* for debug */
    this->counter++;

    // lv_task_handler();

    return false; /**< no abort */
}

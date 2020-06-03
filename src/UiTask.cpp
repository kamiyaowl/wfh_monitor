#include <cfloat>

#include <lvgl.h>

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
    lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(label, "Hello Arduino! (V6.1.1)");
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
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

    // lvgl worker
    lv_task_handler();

    // for debug
    this->counter++;

    return false; /**< no abort */
}

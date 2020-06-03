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
    // TODO: いい感じに置き換え、おそらくlvglのタスク記述だけのクラスが必要
    lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(label, "WHF Monitor on Wio Terminal");
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

    /*Create a chart*/
    lv_obj_t * chart;
    chart = lv_chart_create(lv_scr_act(), NULL);
    lv_obj_set_size(chart, 200, 150);
    lv_obj_align(chart, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);   /*Show lines and points too*/

    /*Add two data series*/
    lv_chart_series_t * ser1 = lv_chart_add_series(chart, LV_COLOR_RED);
    lv_chart_series_t * ser2 = lv_chart_add_series(chart, LV_COLOR_GREEN);

    /*Set the next points on 'ser1'*/
    lv_chart_set_next(chart, ser1, 10);
    lv_chart_set_next(chart, ser1, 10);
    lv_chart_set_next(chart, ser1, 10);
    lv_chart_set_next(chart, ser1, 10);
    lv_chart_set_next(chart, ser1, 10);
    lv_chart_set_next(chart, ser1, 10);
    lv_chart_set_next(chart, ser1, 10);
    lv_chart_set_next(chart, ser1, 30);
    lv_chart_set_next(chart, ser1, 70);
    lv_chart_set_next(chart, ser1, 90);

    /*Directly set points on 'ser2'*/
    ser2->points[0] = 90;
    ser2->points[1] = 70;
    ser2->points[2] = 65;
    ser2->points[3] = 65;
    ser2->points[4] = 65;
    ser2->points[5] = 65;
    ser2->points[6] = 65;
    ser2->points[7] = 65;
    ser2->points[8] = 65;
    ser2->points[9] = 65;

    lv_chart_refresh(chart);
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

#include <cfloat>

#include <lvgl.h>

#include "SysTimer.h"
#include "UiTask.h"

// TODO: ちゃんと配置は考える
static lv_obj_t* chart;
static lv_chart_series_t* s1;
static lv_chart_series_t* s2;
static lv_chart_series_t* s3;
static lv_chart_series_t* s4;
static lv_chart_series_t* s5;

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
    // TODO: ちゃんとやる
    lv_obj_t* tv = lv_tabview_create(lv_scr_act(), NULL);
    lv_obj_t* t1 = lv_tabview_add_tab(tv, "Monitor");
    lv_obj_t* t2 = lv_tabview_add_tab(tv, "Detail");
    lv_obj_t* t3 = lv_tabview_add_tab(tv, "Setting");

    lv_obj_t* parent = t1;

    chart = lv_chart_create(parent, NULL);
    lv_obj_set_drag_parent(chart, true);
    lv_chart_set_div_line_count(chart, 3, 0);
    lv_chart_set_point_count(chart, 8);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_chart_set_y_tick_length(chart, 0, 0);
    lv_chart_set_x_tick_length(chart, 0, 0);
    lv_chart_set_y_tick_texts(chart, "600\n500\n400\n300\n200", 0, LV_CHART_AXIS_DRAW_LAST_TICK);
    s1 = lv_chart_add_series(chart, lv_color_make(200,0,0));
    s2 = lv_chart_add_series(chart, lv_color_make(0,200,0));
    s3 = lv_chart_add_series(chart, lv_color_make(0,0,200));
    s4 = lv_chart_add_series(chart, lv_color_make(200,200,0));
    s5 = lv_chart_add_series(chart, lv_color_make(200,0,200));
}

bool UiTask::loop(void) {
    // receive queue datas
    if (this->recvMeasureDataQueue.remainNum() > 0) {
        this->recvMeasureDataQueue.receive(&this->latestMeasureData, false);
        // test
        // lv_chart_set_next(chart, s1, static_cast<int16_t>(this->latestMeasureData.gas));
        // lv_chart_set_next(chart, s2, static_cast<int16_t>(this->latestMeasureData.humidity));
        // lv_chart_set_next(chart, s3, static_cast<int16_t>(this->latestMeasureData.pressure));
        // lv_chart_set_next(chart, s4, static_cast<int16_t>(this->latestMeasureData.tempature));
        lv_chart_set_next(chart, s5, static_cast<int16_t>(this->latestMeasureData.visibleLux));
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

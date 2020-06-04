/**
 * @file wfh_monitor.ino
 * @author kamiyaowl (kamiyaowl@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2020-06-01
 * 
 * @copyright Copyright (c) 2020 kamiyaowl
 * 
 */
#include <cstdint>

/****************************** Options ******************************/
// #define WFH_MONITOR_ENABLE_SERIAL_PRINT_SENSOR_DATA
// #define WFH_MONITOR_ENABLE_SERIAL_PRINT_BUTTON_DATA
// #define WFH_MONITOR_ENABLE_SERIAL_PRINT_BRIGHTNESS_CONTROL

/****************************** peripheral ******************************/
#include <SPI.h>
#include <Wire.h>

// FreeRTOSで異常発生時はd13のLEDを消灯させる
static constexpr uint32_t errorLedPinNum = 13;
static constexpr uint32_t errorLedState =  LOW;

static TwoWire &wireL = Wire;  // left  port

/****************************** Hardware Library ******************************/
#include <LovyanGFX.hpp>
#include <Digital_Light_TSL2561.h>
#include <seeed_bme680.h>
#include <Seeed_Arduino_FreeRTOS.h>

static LGFX lcd;               
static LGFX_Sprite sprite(&lcd);
static TSL2561_CalculateLux &lightSensor = TSL2561; // TSL2561 Digital Light Sensor
static Seeed_BME680 bme680((uint8_t)0x76);          // BME680 SlaveAddr=0x76

/****************************** LVGL ******************************/
// reference: https://docs.lvgl.io/v7/en/html/porting/display.html
#include <lvgl.h>

static constexpr size_t lvglBufSize = LV_HOR_RES_MAX * 10;
static lv_disp_buf_t dispBuf;
static lv_color_t buf1[lvglBufSize]; // foreground buf
static lv_color_t buf2[lvglBufSize]; // background buf(optional)

/****************************** RTOS Queue ******************************/
#include "src/IpcQueueDefs.h"
#include "src/IpcQueue.h"

// 複数CPUで動作させる場合、ローカル変数がCPU Data Cacheに乗る可能性があるので
// NonCacheアクセスを矯正できる場所(TCM), 参照時はNonCacheアクセスする, 書き込み後FlushDCache/読み出し前InvalidateDCacheを徹底する
static IpcQueue<MeasureData> measureDataQueue;
static IpcQueue<ButtonEventData> buttonStateQueue;

/****************************** RTOS Task ******************************/
#include "src/TaskBase.h"
#include "src/GroveTask.h"
#include "src/ButtonTask.h"
#include "src/UiTask.h"

static GroveTask groveTask(measureDataQueue, Serial, lightSensor, bme680);
static ButtonTask<2> buttonTask(buttonStateQueue, Serial);
static UiTask uiTask(measureDataQueue, buttonStateQueue, Serial, lcd, sprite);

/****************************** Setup Subfunction ******************************/

/**
 * @brief LVGLを初期化します
 */
static void setupLvgl(void) {
    lv_init();
    lv_disp_buf_init(&dispBuf, buf1, buf2, LV_HOR_RES_MAX * 10);

    // initialize display
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 320;
    disp_drv.ver_res = 240;
    disp_drv.buffer = &dispBuf;
    disp_drv.flush_cb = [](lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
        const uint32_t w = (area->x2 - area->x1 + 1);
        const uint32_t h = (area->y2 - area->y1 + 1);

        lcd.startWrite();
        lcd.setAddrWindow(area->x1, area->y1, w, h);
        lcd.pushColors(&color_p->full, w * h, true);
        lcd.endWrite();

        lv_disp_flush_ready(disp);
    };
    lv_disp_drv_register(&disp_drv);
}

/****************************** Main ******************************/
void setup() {
    // for por
    vNopDelayMS(1000);

    // setup peripheral
    Serial.begin(115200);
    wireL.begin();

    // setup display
    // Grove Sensorとは異なりWio Terminalに付随しているHWなのでTask起動前に初期化する
    lcd.begin();
    lcd.setRotation(1);
    setupLvgl();

    // setup rtos queue
    measureDataQueue.createQueue(4);
    buttonStateQueue.createQueue(4);

    // setup rtos task
    vSetErrorLed(errorLedPinNum, errorLedState);
    groveTask.createTask(256, tskIDLE_PRIORITY + 0);
    buttonTask.createTask(256, tskIDLE_PRIORITY + 0);
    uiTask.createTask(1024, tskIDLE_PRIORITY + 1);
    vTaskStartScheduler();
}

void loop() {
}
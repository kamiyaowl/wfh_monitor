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

/****************************** Options ******************************/
// #define WFH_MONITOR_ENABLE_SERIAL_PRINT_SENSOR_DATA
// #define WFH_MONITOR_ENABLE_SERIAL_PRINT_BUTTON_DATA
// #define WFH_MONITOR_ENABLE_SERIAL_PRINT_BRIGHTNESS_CONTROL

/****************************** peripheral ******************************/
#include <SPI.h>
#include <Wire.h>

#define ERROR_LED_PIN  13
#define ERROR_LED_LIGHTUP_STATE LOW

static TwoWire &wireL = Wire;  // left  port

/****************************** Library ******************************/
#include <LovyanGFX.hpp>
#include <Digital_Light_TSL2561.h>
#include <seeed_bme680.h>
#include <Seeed_Arduino_FreeRTOS.h>
#include <lvgl.h>

static LGFX lcd;               
static LGFX_Sprite sprite(&lcd);
static TSL2561_CalculateLux &lightSensor = TSL2561; // TSL2561 Digital Light Sensor
static Seeed_BME680 bme680((uint8_t)0x76);          // BME680 SlaveAddr=0x76
static lv_disp_buf_t dispBuf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

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

/****************************** Test ******************************/


/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    lcd.startWrite();
    lcd.setAddrWindow(area->x1, area->y1, w, h);
    lcd.pushColors(&color_p->full, w * h, true);
    lcd.endWrite();

    lv_disp_flush_ready(disp);
}

/* Reading input device (simulated encoder here) */
bool read_encoder(lv_indev_drv_t * indev, lv_indev_data_t * data)
{
    static int32_t last_diff = 0;
    int32_t diff = 0; /* Dummy - no movement */
    int btn_state = LV_INDEV_STATE_REL; /* Dummy - no press */

    data->enc_diff = diff - last_diff;;
    data->state = btn_state;

    last_diff = diff;

    return false;
}

void setup()
{

    Serial.begin(115200); /* prepare for possible serial debug */

    lv_init();

    lcd.begin(); /* TFT init */
    lcd.setRotation(1); /* Landscape orientation */

    lv_disp_buf_init(&dispBuf, buf, NULL, LV_HOR_RES_MAX * 10);

    /*Initialize the display*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 320;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.buffer = &dispBuf;
    lv_disp_drv_register(&disp_drv);

    /*Initialize the (dummy) input device driver*/
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = read_encoder;
    lv_indev_drv_register(&indev_drv);

    /* Create simple label */
    lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(label, "Hello Arduino! (V6.1.1)");
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
}


void loop()
{

    lv_task_handler(); /* let the GUI do its work */
    delay(5);
}

/****************************** Main ******************************/

// void setup() {
//     // setup peripheral
//     Serial.begin(115200);
//     wireL.begin();

//     // for por
//     vNopDelayMS(1000);

//     // setup rtos queue
//     measureDataQueue.createQueue(4);
//     buttonStateQueue.createQueue(4);

//     // setup rtos task
//     vSetErrorLed(ERROR_LED_PIN, ERROR_LED_LIGHTUP_STATE);
//     groveTask.createTask(  256, tskIDLE_PRIORITY + 0);
//     buttonTask.createTask( 256, tskIDLE_PRIORITY + 0);
//     uiTask.createTask(     256, tskIDLE_PRIORITY + 1);
//     vTaskStartScheduler();
// }

// void loop() {
// }
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
static constexpr char* configPath = "wfhm.json";

/****************************** peripheral ******************************/
#include <SPI.h>
#include <Wire.h>

// FreeRTOSで異常発生時はd13のLEDを消灯させる
static constexpr uint32_t errorLedPinNum = 13;
static constexpr uint32_t errorLedState =  LOW;

static Serial_ serial = Serial; // to pc communicate
static TwoWire& wireL = Wire;  // left  port

/****************************** Hardware Library ******************************/
#include <LovyanGFX.hpp>
#include <Digital_Light_TSL2561.h>
#include <seeed_bme680.h>
#include <Seeed_Arduino_FreeRTOS.h>
#include <Seeed_FS.h>
#include "SD/Seeed_SD.h"

static LGFX lcd;               
static LGFX_Sprite sprite(&lcd);
static TSL2561_CalculateLux &lightSensor = TSL2561; // TSL2561 Digital Light Sensor
static Seeed_BME680 bme680((uint8_t)0x76);          // BME680 SlaveAddr=0x76
static SDFS& sd = SD;

/****************************** RTOS Queue ******************************/
#include "src/IpcQueueDefs.h"
#include "src/IpcQueue.h"

// 複数CPUで動作させる場合、ローカル変数がCPU Data Cacheに乗る可能性があるので
// NonCacheアクセスを矯正できる場所(TCM), 参照時はNonCacheアクセスする, 書き込み後FlushDCache/読み出し前InvalidateDCacheを徹底する
static IpcQueue<MeasureData> measureDataQueue;
static IpcQueue<ButtonEventData> buttonStateQueue;

/****************************** RTOS SharedData ******************************/
#include "src/SharedResource.h"
#include "src/GlobalConfig.h"

// RTOS Queueと同様semaphoreHandleがCPU DataCache上に配置されることを回避すること
static SharedResource<Serial_> sharedSerial(serial);
static SharedResource<SDFS> sharedSd(sd);

static GlobalConfig config(sharedSd, configPath);
static SharedResource<GlobalConfig> sharedConfig(config);

/****************************** RTOS Task ******************************/
#include "src/TaskBase.h"
#include "src/GroveTask.h"
#include "src/ButtonTask.h"
#include "src/UiTask.h"

static GroveTask groveTask(measureDataQueue, lightSensor, bme680);
static ButtonTask<2> buttonTask(buttonStateQueue);
static UiTask uiTask(measureDataQueue, buttonStateQueue, lcd, sprite);

/****************************** Setup Subfunction ******************************/

void setup() {
    // for por
    delay(1000);

    /* setup implemented HW */
    // setup peripheral
    Serial.begin(115200);
    wireL.begin();

    // setup display
    lcd.begin();
    lcd.setRotation(1);

    // setup sd
    sd.begin(SDCARD_SS_PIN, SDCARD_SPI);
    // test
    config.save(nullptr);

    /* setup RTOS */
    // setup rtos queue
    measureDataQueue.createQueue(4);
    buttonStateQueue.createQueue(4);

    // setup rtos task
    vSetErrorLed(errorLedPinNum, errorLedState);
    groveTask.createTask(256, tskIDLE_PRIORITY + 0);
    buttonTask.createTask(256, tskIDLE_PRIORITY + 0);
    uiTask.createTask(4096, tskIDLE_PRIORITY + 1);
    vTaskStartScheduler();
}

void loop() {
}
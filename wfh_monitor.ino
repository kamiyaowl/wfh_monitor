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
static constexpr uint32_t waitForPorMs        = 1000;
static constexpr uint32_t waitForDebugPrintMs = 1000;
static constexpr char*    configPath          = "wfhm.json";
static constexpr uint32_t configAllocateSize  = 1024;

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
#include <ArduinoJson.h>
#include "src/SharedResource.h"
#include "src/GlobalConfig.h"

// RTOS Queueと同様semaphoreHandleがCPU DataCache上に配置されることを回避すること
static SharedResource<Serial_> sharedSerial(serial);
static SharedResource<SDFS> sharedSd(sd);

static GlobalConfig<configAllocateSize> config(sharedSd, configPath);
static SharedResource<GlobalConfig<configAllocateSize>> sharedConfig(config);

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
    delay(waitForPorMs);

    // setup display
    lcd.begin();
    lcd.setTextSize(1);
    lcd.setRotation(1);
    lcd.println("boot WFH Monitor...\n");

    /* setup implemented HW */
    lcd.println("[INFO] setup peripheral\n");
    Serial.begin(115200);
    wireL.begin();

    // setup sd
    lcd.println("[INFO] setup SD card\n");
    sd.begin(SDCARD_SS_PIN, SDCARD_SPI);
    
    /* load configure from SD card */
    lcd.println("[INFO] load config from SD card\n");

    DeserializationError desError; // TODO: Error表示を出すか検討
    if (config.load(nullptr, desError)) {
        lcd.println("[INFO] done.\n");
    } else {
        lcd.println("[ERROR] failed, init default value.\n");
        // initialize and save to SD card
        config.init(false);

        lcd.println("[INFO] save config to SD card\n");
        if (config.save(nullptr)) {
            lcd.println("[INFO] done.\n");
        } else {
            lcd.println("[ERROR] failed.\n");
        }
    }

    /* setup RTOS */
    lcd.println("[INFO] setup RTOS config\n");
    vSetErrorLed(errorLedPinNum, errorLedState);

    lcd.println("[INFO] setup RTOS queue\n");
    measureDataQueue.createQueue(4);
    buttonStateQueue.createQueue(4);

    lcd.println("[INFO] setup RTOS task\n");
    groveTask.createTask(256, tskIDLE_PRIORITY + 0);
    buttonTask.createTask(256, tskIDLE_PRIORITY + 0);
    uiTask.createTask(4096, tskIDLE_PRIORITY + 1);

    /* keep debug print */
    lcd.println("[INFO] done.\n");
    delay(waitForDebugPrintMs);
    lcd.clear();

    /* start task */
    vTaskStartScheduler();
}

void loop() {
}
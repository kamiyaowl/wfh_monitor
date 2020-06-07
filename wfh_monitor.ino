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

// Projectで固定されたコンパイル時定数
#include "src/FixedConfig.h"

/****************************** Peripheral ******************************/
#include <SPI.h>
#include <Wire.h>

static Serial_ serial = Serial;
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
static Seeed_BME680 bme680(FixedConfig::Bme680SlaveAddr);          // BME680 SlaveAddr=0x76
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
#include "src/SharedResourceDefs.h"
#include "src/GlobalConfig.h"

// RTOS Queueと同様semaphoreHandleがCPU DataCache上に配置されることを回避すること
static SharedResource<Serial_> sharedSerial(serial);
static SharedResource<SDFS> sharedSd(sd);
// configも共有する、load/saveにSDFSが必要
static GlobalConfig<FixedConfig::ConfigAllocateSize> config(sharedSd, FixedConfig::ConfigPath);
static SharedResource<GlobalConfig<FixedConfig::ConfigAllocateSize>> sharedConfig(config);
// 他Taskに公開するResouceを記述
static SharedResourceDefs sharedResources = {
    .serial = sharedSerial,
    .sd     = sharedSd,
    .config = sharedConfig,
};

/****************************** RTOS Task ******************************/
#include "src/TaskBase.h"
#include "src/GroveTask.h"
#include "src/ButtonTask.h"
#include "src/UiTask.h"

static GroveTask groveTask(sharedResources, measureDataQueue, lightSensor, bme680);
static ButtonTask<FixedConfig::ButtonTaskDebounceNum> buttonTask(sharedResources, buttonStateQueue);
static UiTask<FixedConfig::UiTaskBrightnessKeyPoint> uiTask(sharedResources, measureDataQueue, buttonStateQueue, lcd, sprite);

/****************************** Setup Subfunction ******************************/

void setup() {
    // for por
    delay(FixedConfig::WaitForPorMs);

    // setup display
    lcd.begin();
    lcd.setTextSize(1);
    lcd.setRotation(1);
    lcd.printf("boot WFH Monitor...\n");

    /* setup implemented HW */
    lcd.printf("[INFO] setup peripheral\n");
    wireL.begin();
    Serial.begin(FixedConfig::SerialBaudrate);
    // USB UARTが準備できるまで待つ
    while (FixedConfig::WaitForInitSerial && !Serial) {
        lcd.print(".");
        delay(FixedConfig::WaitForPorMs);
    }

    // setup sd
    lcd.printf("[INFO] setup SD card\n");
    sd.begin(SDCARD_SS_PIN, SDCARD_SPI);
    
    /* load configure from SD card */
    lcd.printf("[INFO] load config from SD card\n");

    DeserializationError desError;
    if (config.load(nullptr, desError)) {
        lcd.printf("[INFO] done.\n");
    } else {
        lcd.printf("[ERROR] failed code=%d, init default value.\n", desError);
        // initialize and save to SD card
        config.init(false);

        lcd.printf("[INFO] save config to SD card\n");
        if (config.save(nullptr)) {
            lcd.printf("[INFO] done.\n");
        } else {
            lcd.printf("[ERROR] failed.\n");
        }
    }

    /* setup RTOS */
    lcd.printf("[INFO] setup RTOS config\n");
    vSetErrorLed(FixedConfig::ErrorLedPinNum, FixedConfig::ErrorLedState);

    lcd.printf("[INFO] setup RTOS queue\n");
    measureDataQueue.createQueue(FixedConfig::DefaultQueueSize);
    buttonStateQueue.createQueue(FixedConfig::DefaultQueueSize);

    lcd.printf("[INFO] setup RTOS task\n");
    groveTask.createTask(FixedConfig::GroveTaskStackSize, tskIDLE_PRIORITY + 0);
    buttonTask.createTask(FixedConfig::ButtonTaskStackSize, tskIDLE_PRIORITY + 0);
    uiTask.createTask(FixedConfig::UiTaskStackSize, tskIDLE_PRIORITY + 1);

    /* keep debug print */
    lcd.printf("[INFO] done.\n");
    delay(FixedConfig::WaitForDebugPrintMs);
    lcd.clear();

    /* start task */
    vTaskStartScheduler();
}

void loop() {
}
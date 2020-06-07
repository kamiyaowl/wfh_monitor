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

/****************************** Config(Fixed) ******************************/
/**
 * @brief コンパイル時に設定が必要な定数群です
 */
namespace FixedConfig {
    static constexpr uint8_t  Bme680SlaveAddr          = 0x76;          /**< BME680のSlave Addr */
    static constexpr uint32_t WaitForPorMs             = 1000;          /**< POR後の待機時間 */
    static constexpr uint32_t SerialBaudrate           = 115200;        /**< UART baudrate */
    static constexpr char*    ConfigPath               = "wfhm.json";   /**< SD Cardのconfig保存先 */
    static constexpr size_t   ConfigAllocateSize       = 1024;          /**< config格納用に使用する領域サイズ(configの内容が大きい場合は要調整) */
    static constexpr uint32_t ErrorLedPinNum           = 13;            /**< RTOSでエラー発生時のLED Pin番号 */
    static constexpr uint32_t ErrorLedState            = LOW;           /**< RTOSでエラー発生時のLEDの状態 */
    static constexpr size_t   DefaultQueueSize         = 4;             /**< SensorData/ButtonStateのQueue Size */
    static constexpr size_t   GroveTaskStackSize       = 256;           /**< GroveTaskのStackSize */
    static constexpr size_t   ButtonTaskStackSize      = 256;           /**< ButtonTaskのStackSize */
    static constexpr size_t   UiTaskStackSize          = 4096;          /**< UiTaskのStackSize */
    static constexpr uint32_t WaitForDebugPrintMs      = 1000;          /**< Task開始直前の待機時間 */
    static constexpr size_t   ButtonTaskDebounceNum    = 2;             /**< ButtonTaskで保持する履歴数 */
    static constexpr size_t   UiTaskBrightnessKeyPoint = 4;             /**< 画面自動調光の設定KeyPoint数 */
}

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
#include "src/GlobalConfig.h"

// RTOS Queueと同様semaphoreHandleがCPU DataCache上に配置されることを回避すること
static SharedResource<Serial_> sharedSerial(serial);
static SharedResource<SDFS> sharedSd(sd);

static GlobalConfig<FixedConfig::ConfigAllocateSize> config(sharedSd, FixedConfig::ConfigPath);
static SharedResource<GlobalConfig<FixedConfig::ConfigAllocateSize>> sharedConfig(config);

/****************************** RTOS Task ******************************/
#include "src/TaskBase.h"
#include "src/GroveTask.h"
#include "src/ButtonTask.h"
#include "src/UiTask.h"

static GroveTask groveTask(measureDataQueue, lightSensor, bme680);
static ButtonTask<FixedConfig::ButtonTaskDebounceNum> buttonTask(buttonStateQueue);
static UiTask<FixedConfig::UiTaskBrightnessKeyPoint> uiTask(measureDataQueue, buttonStateQueue, lcd, sprite);

/****************************** Setup Subfunction ******************************/

void setup() {
    // for por
    delay(FixedConfig::WaitForPorMs);

    // setup display
    lcd.begin();
    lcd.setTextSize(1);
    lcd.setRotation(1);
    lcd.println("boot WFH Monitor...\n");

    /* setup implemented HW */
    lcd.println("[INFO] setup peripheral\n");
    Serial.begin(FixedConfig::SerialBaudrate);
    wireL.begin();

    // setup sd
    lcd.println("[INFO] setup SD card\n");
    sd.begin(SDCARD_SS_PIN, SDCARD_SPI);
    
    /* load configure from SD card */
    lcd.println("[INFO] load config from SD card\n");

    DeserializationError desError;
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
    vSetErrorLed(FixedConfig::ErrorLedPinNum, FixedConfig::ErrorLedState);

    lcd.println("[INFO] setup RTOS queue\n");
    measureDataQueue.createQueue(FixedConfig::DefaultQueueSize);
    buttonStateQueue.createQueue(FixedConfig::DefaultQueueSize);

    lcd.println("[INFO] setup RTOS task\n");
    groveTask.createTask(FixedConfig::GroveTaskStackSize, tskIDLE_PRIORITY + 0);
    buttonTask.createTask(FixedConfig::ButtonTaskStackSize, tskIDLE_PRIORITY + 0);
    uiTask.createTask(FixedConfig::UiTaskStackSize, tskIDLE_PRIORITY + 1);

    /* keep debug print */
    lcd.println("[INFO] done.\n");
    delay(FixedConfig::WaitForDebugPrintMs);
    lcd.clear();

    /* start task */
    vTaskStartScheduler();
}

void loop() {
}
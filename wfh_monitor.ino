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
#include <AtWiFi.h>

static LGFX lcd;               
static LGFX_Sprite sprite(&lcd);
static TSL2561_CalculateLux &lightSensor = TSL2561; // TSL2561 Digital Light Sensor
static Seeed_BME680 bme680(FixedConfig::Bme680SlaveAddr);
static SDFS& sd = SD;
static WiFiClass wifi = WiFi;

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
static void setupLcd(void) {
    lcd.begin();
    lcd.setTextSize(1);
    lcd.setRotation(1);
    lcd.printf("boot WFH Monitor...\n");
}

static void setupPeripheral(void) {
    lcd.printf("[INFO] setup peripheral\n");
    wireL.begin();
    Serial.begin(FixedConfig::SerialBaudrate);
    // USB UARTが準備できるまで待つオプションを有効にしてビルドした場合
    while (FixedConfig::WaitForInitSerial && !Serial) {
        lcd.print(".");
        delay(FixedConfig::WaitForPorMs);
    }
}

static void setupSd(void) {
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
}

static void setupWifi(void) {
    // 前回動いていた場合、一旦切断
    lcd.printf("[INFO] reset wifi module.\n");
    wifi.mode(WIFI_STA);
    wifi.disconnect();
    delay(FixedConfig::WaitForPorMs);

    // WiFi使わなければSkip
    bool useWifi = GlobalConfigDefaultValues::UseWiFi;
    config.read(GlobalConfigKeys::UseWiFi, useWifi);
    if (!useWifi) {
        lcd.printf("[INFO] skip AP Connection.\n");
        return;
    }

    // Wifi開始
    const char* ssid   = "ssid"; //TODO: #49 config.getReadPtr<char>(GlobalConfigKeys::ApSsid);
    const char* pass   = "pass"; //TODO: #49 config.getReadPtr<char>(GlobalConfigKeys::ApPassWord);
    uint32_t timeoutMs = GlobalConfigDefaultValues::ApTimeoutMs;
    config.read(GlobalConfigKeys::ApTimeoutMs, timeoutMs);

    lcd.printf("[INFO] connect to %s.\n", ssid);
    wifi.begin(ssid, pass);

    const uint32_t startMillis = millis();
    while (wifi.status() != WL_CONNECTED) {
        lcd.print(".");
        delay(FixedConfig::WaitForPorMs);
        // timeout
        const uint32_t elapsedMs = millis() - startMillis;
        if (elapsedMs > timeoutMs) {
            lcd.printf("\n[ERROR] timeout.\n");
            break;
        }
    }

    const bool isConnected = (wifi.status() == WL_CONNECTED);
    if (!isConnected) {
        lcd.printf("[ERROR] Error.\n");
        return;
    }

    lcd.printf("\n[INFO] done.");
    lcd.println(wifi.localIP());
}

static void setupRtos(void) {
    lcd.printf("[INFO] setup RTOS config\n");
    vSetErrorLed(FixedConfig::ErrorLedPinNum, FixedConfig::ErrorLedState);

    lcd.printf("[INFO] setup RTOS queue\n");
    measureDataQueue.createQueue(FixedConfig::DefaultQueueSize);
    buttonStateQueue.createQueue(FixedConfig::DefaultQueueSize);

    /* WiFiですでにRTOSが動いているので一旦止める */
    lcd.printf("[INFO] done. wait=%d[ms]\n", FixedConfig::WaitForDebugPrintMs);
    delay(FixedConfig::WaitForDebugPrintMs);
    lcd.clear();

    /* Task開始: 以後はTask以外の操作は基本行わない */
    groveTask.createTask(FixedConfig::GroveTaskStackSize, configMAX_PRIORITIES - 1);
    buttonTask.createTask(FixedConfig::ButtonTaskStackSize, configMAX_PRIORITIES - 1);
    uiTask.createTask(FixedConfig::UiTaskStackSize, configMAX_PRIORITIES - 0);

    /* AtWiFiに依存する部分がすでにいくつかのTaskを動かしているので開始操作は不要 */
}

/****************************** Main ******************************/
void setup() {
    // for por
    delay(FixedConfig::WaitForPorMs);

    /* setup implemented HW */
    setupLcd();
    setupPeripheral();
    setupSd(); 
    setupWifi();   

    /* setup RTOS and Run */
    setupRtos();
}

void loop() {
}
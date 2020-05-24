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

static LGFX lcd;               
static LGFX_Sprite sprite(&lcd);
static TSL2561_CalculateLux &lightSensor = TSL2561; // TSL2561 Digital Light Sensor
static Seeed_BME680 bme680((uint8_t)0x76);          // BME680 SlaveAddr=0x76

/****************************** RTOS Task ******************************/
#define WFH_MONITOR_ENABLE_DEBUG (1) /**< for debug mode*/
#include "TaskBase.h"
#include "GroveTask.h"
#include "ButtonTask.h"
#include "UiTask.h"

static GroveTask groveTask(Serial, lightSensor, bme680);
static ButtonTask buttonTask(Serial);
static UiTask uiTask(Serial, lcd, sprite);

/****************************** Main ******************************/
void setup() {
    // setup peripheral
    Serial.begin(115200);
    wireL.begin();

    // for por
    vNopDelayMS(1000);

    // setup rtos
    vSetErrorLed(ERROR_LED_PIN, ERROR_LED_LIGHTUP_STATE);
    groveTask.createTask(256, tskIDLE_PRIORITY);
    buttonTask.createTask(256, tskIDLE_PRIORITY);
    uiTask.createTask(256, tskIDLE_PRIORITY);
    vTaskStartScheduler();
}

void loop() {
    // vNopDelayMS(100);
}
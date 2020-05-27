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

/****************************** RTOS Queue ******************************/
#include "IpcQueueDefs.h"
#include "IpcQueue.h"

// 複数CPUで動作させる場合、ローカル変数がCPU Data Cacheに乗る可能性があるので
// NonCacheアクセスを矯正できる場所(TCM), 参照時はNonCacheアクセスする, 書き込み後FlushDCache/読み出し前InvalidateDCacheを徹底する
static IpcQueue<MeasureData_t> measureDataQueue;
static IpcQueue<ButtonStateBmp_t> buttonStateQueue;

/****************************** RTOS Task ******************************/
#define WFH_MONITOR_ENABLE_SERIAL_PRINT_SENSOR_DATA (0)
#define WFH_MONITOR_ENABLE_SERIAL_PRINT_BUTTON_DATA (1)

#include "TaskBase.h"
#include "GroveTask.h"
#include "ButtonTask.h"
#include "UiTask.h"

static GroveTask groveTask(measureDataQueue, Serial, lightSensor, bme680);
static ButtonTask buttonTask(buttonStateQueue, Serial);
static UiTask uiTask(measureDataQueue, buttonStateQueue, Serial, lcd, sprite);

/****************************** Main ******************************/
void setup() {
    // setup peripheral
    Serial.begin(115200);
    wireL.begin();

    // for por
    vNopDelayMS(1000);

    // setup rtos queue
    measureDataQueue.createQueue(4);
    buttonStateQueue.createQueue(4);

    // setup rtos task
    vSetErrorLed(ERROR_LED_PIN, ERROR_LED_LIGHTUP_STATE);
    groveTask.createTask(  256, tskIDLE_PRIORITY + 0);
    buttonTask.createTask( 256, tskIDLE_PRIORITY + 0);
    uiTask.createTask(     256, tskIDLE_PRIORITY + 0);
    vTaskStartScheduler();
}

void loop() {
    // vNopDelayMS(100);
}
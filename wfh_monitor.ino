#include <SPI.h>
#include <Wire.h>

static TwoWire &wireL = Wire;  // left  port

#include <LovyanGFX.hpp>
#include <Digital_Light_TSL2561.h>
#include <seeed_bme680.h>
#include <Seeed_Arduino_FreeRTOS.h>

#include "TaskBase.h"
#include "GroveTask.h"

static LGFX lcd;               
static LGFX_Sprite sprite(&lcd);
static TSL2561_CalculateLux &lightSensor = TSL2561; // TSL2561 Digital Light Sensor
static Seeed_BME680 bme680((uint8_t)0x76);                   // BME680 SlaveAddr=0x76

#define ERROR_LED_PIN  13
#define ERROR_LED_LIGHTUP_STATE LOW

static TaskHandle_t Handle_aTask;
static TaskHandle_t Handle_bTask;
static TaskHandle_t Handle_monitorTask;
static TaskHandle_t groveTaskHandle;

void myDelayUs(int us) {
    vTaskDelay(us / portTICK_PERIOD_US);
}

void myDelayMsUntil(TickType_t* previousWakeTime, int ms) {
    vTaskDelayUntil(previousWakeTime, (ms * 1000) / portTICK_PERIOD_US);
}

static void threadA(void* pvParameters) {

    Serial.println("Thread A: Started");
    for (int x = 0; x < 20; ++x) {
        Serial.print("A");
        delay(500);
    }

    // delete ourselves.
    // Have to call this or the system crashes when you reach the end bracket and then get scheduled.
    Serial.println("Thread A: Deleting");
    vTaskDelete(NULL);
}

static void threadB(void* pvParameters) {
    Serial.println("Thread B: Started");

    while (1) {
        Serial.println("B");
        delay(2000);
    }

}

void taskMonitor(void* pvParameters) {
    int x;
    int measurement;

    Serial.println("Task Monitor: Started");

    // run this task afew times before exiting forever
    for (x = 0; x < 10; ++x) {

        Serial.println("");
        Serial.println("******************************");
        Serial.println("[Stacks Free Bytes Remaining] ");

        measurement = uxTaskGetStackHighWaterMark(Handle_aTask);
        Serial.print("Thread A: ");
        Serial.println(measurement);

        measurement = uxTaskGetStackHighWaterMark(Handle_bTask);
        Serial.print("Thread B: ");
        Serial.println(measurement);

        measurement = uxTaskGetStackHighWaterMark(Handle_monitorTask);
        Serial.print("Monitor Stack: ");
        Serial.println(measurement);

        Serial.println("******************************");

        delay(10000); // print every 10 seconds
    }

    // delete ourselves.
    // Have to call this or the system crashes when you reach the end bracket and then get scheduled.
    Serial.println("Task Monitor: Deleting");
    vTaskDelete(NULL);

}


//*****************************************************************

void setup() {
    Serial.begin(115200);

    vNopDelayMS(1000);

    Serial.println("");
    Serial.println("******************************");
    Serial.println("        Program start         ");
    Serial.println("******************************");

    vSetErrorLed(ERROR_LED_PIN, ERROR_LED_LIGHTUP_STATE);

    xTaskCreate(threadB,     "Task B",       256, NULL, tskIDLE_PRIORITY + 2, &Handle_bTask);
    xTaskCreate(threadB,     "Task B",       256, NULL, tskIDLE_PRIORITY + 2, &Handle_bTask);
    xTaskCreate(taskMonitor, "Task Monitor", 256, NULL, tskIDLE_PRIORITY + 1, &Handle_monitorTask);

    // Start the RTOS, this function will never return and will schedule the tasks.
    vTaskStartScheduler();

}

//*****************************************************************
// This is now the rtos idle loop
// No rtos blocking functions allowed!
//*****************************************************************
void loop() {
    // Optional commands, can comment/uncomment below
    Serial.print("."); //print out dots in terminal, we only do this when the RTOS is in the idle state
    vNopDelayMS(100);
}


// void setup() {
//     // for POR
//     delay(1000);

//     // setup peripheral 
//     Serial.begin(9600);
//     wireL.begin();

//     // setup modules
//     lcd.init();
//     lightSensor.init();
//     bme680.init();
// }

// static uint32_t loopCounter = 0;
// void loop() {
//     bme680.read_sensor_data();
//     const float   tempature  = bme680.sensor_result_value.temperature;
//     const float   pressure   = bme680.sensor_result_value.pressure / 1000.0f;
//     const float   humidity   = bme680.sensor_result_value.humidity;  
//     const float   gas        = bme680.sensor_result_value.gas / 1000.0f;
//     const int32_t visibleLux = lightSensor.readVisibleLux();

//     Serial.print(visibleLux);
//     Serial.print(",");
//     Serial.print(tempature);
//     Serial.print(",");
//     Serial.print(pressure);
//     Serial.print(",");
//     Serial.print(humidity);
//     Serial.print(",");
//     Serial.print(gas);
//     Serial.print(",");
//     Serial.println("");

//     lcd.setCursor(0, 0);
//     lcd.printf("counter=%d\n", loopCounter);
//     lcd.printf("visibleLux=%03d\n", visibleLux);
//     lcd.printf("tempature=%f\n", tempature);
//     lcd.printf("pressure=%f\n", pressure);
//     lcd.printf("humidity=%f\n", humidity);
//     lcd.printf("gas=%f\n", gas);

//     loopCounter++;
// }

#include <SPI.h>
#include <Wire.h>

static TwoWire &wireL = Wire;  // left  port

#include <LovyanGFX.hpp>
#include <Digital_Light_TSL2561.h>
#include <seeed_bme680.h>

static LGFX lcd;               
static LGFX_Sprite sprite(&lcd);
static TSL2561_CalculateLux &lightSensor = TSL2561; // TSL2561 Digital Light Sensor
static Seeed_BME680 bme680((uint8_t)0x76);                   // BME680 SlaveAddr=0x76

void setup() {
    // for POR
    delay(1000);

    // setup peripheral 
    Serial.begin(9600);
    wireL.begin();

    // setup modules
    lcd.init();
    lightSensor.init();
    bme680.init();
}

static uint32_t loopCounter = 0;
void loop() {
    bme680.read_sensor_data();
    const float   tempature  = bme680.sensor_result_value.temperature;
    const float   pressure   = bme680.sensor_result_value.pressure / 1000.0f;
    const float   humidity   = bme680.sensor_result_value.humidity;  
    const float   gas        = bme680.sensor_result_value.gas / 1000.0f;
    const int32_t visibleLux = lightSensor.readVisibleLux();

    Serial.print(visibleLux);
    Serial.print(",");
    Serial.print(tempature);
    Serial.print(",");
    Serial.print(pressure);
    Serial.print(",");
    Serial.print(humidity);
    Serial.print(",");
    Serial.print(gas);
    Serial.print(",");
    Serial.println("");

    lcd.setCursor(0, 0);
    lcd.printf("counter=%d\n", loopCounter);
    lcd.printf("visibleLux=%03d\n", visibleLux);
    lcd.printf("tempature=%f\n", tempature);
    lcd.printf("pressure=%f\n", pressure);
    lcd.printf("humidity=%f\n", humidity);
    lcd.printf("gas=%f\n", gas);

    loopCounter++;
}

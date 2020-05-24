#include <SPI.h>
#include <Wire.h>

#include <LovyanGFX.hpp>
#include <Digital_Light_TSL2561.h>
#include <seeed_bme680.h>

static TwoWire &wireL = Wire; // left  port

static LGFX lcd;               
static LGFX_Sprite sprite(&lcd);
static TSL2561_CalculateLux &lightSensor = TSL2561; // TSL2561 Digital Light Sensor
static Seeed_BME680 bme680(0x76);                   // BME680 SlaveAddr=0x76

void setup() {
    // for POR
    delay(1000);

    // setup peripheral 
    Serial.begin(9600);
    wireL.begin();
    wireR.begin();

    // setup modules
    lcd.init();
    lightSensor.init();
    bme680.init();
}

static uint32_t loopCounter = 0;
void loop() {
    const int32_t visibleLux = lightSensor.readVisibleLux();

    Serial.print(visibleLux);
    Serial.println(",");

    lcd.setCursor(0, 0);
    lcd.printf("counter=%d", loopCounter);
    lcd.println("");
    lcd.printf("visibleLux=%03d", visibleLux);
    lcd.println("");

    loopCounter++;
}

#include <Wire.h>

/**
 * @brief 指定されたI2C Slave Deviceのレジスタからデータを読み出します
 * 
 * @param wire 対象のI2C Peripheral
 * @param slaveAddr 対象のSlave Address
 * @param regAddr 読み出したい先頭のアドレス
 * @param regDataPtr 読みだしたデータの格納先のポインタ。readByteCount分は確保されている必要があります
 * @param readByteCount 読みだしbyte数
 * @return uint8_t Error Code。正常に完了した場合はI2C_ERROR_OK(0)が返ります
 * 
 * @note regAddrのオートインクリメントにデバイスが対応している必要があります
 */
uint8_t i2cReadReg(TwoWire &wire, uint8_t slaveAddr, uint8_t regAddr, uint8_t *regDataPtr, uint16_t readByteCount) {
    // readByteCount=0の場合、通信する必要がない
    if (readByteCount == 0) {
        return 0;
    }

    for (uint16_t i = 0; i < readByteCount; i++) {
        // Write regAddr for read
        wire.beginTransmission(slaveAddr);
        wire.write(regAddr + i);
        const uint8_t errorCode = Wire.endTransmission(false); // don't send stopbit

        // regAddrを書き込めてなければ処理中断
        if (errorCode != 0) {
            return errorCode;
        }

        // Read reg
        const uint8_t readBytes = wire.requestFrom(slaveAddr, 1, true); // restart, read and send stopbit
        regDataPtr[i] = wire.read();
    }

#if 0 /* I2CでBurst Readかけると何故かStopbit待ちになってハングする */
    // Write regAddr for read
    wire.beginTransmission(slaveAddr);
    wire.write(regAddr);
    const uint8_t errorCode = Wire.endTransmission(false); // don't send stopbit

    // regAddrを書き込めてなければ処理中断
    if (errorCode != 0) {
        return errorCode;
    }

    // Read regs
    const uint8_t readBytes = wire.requestFrom(slaveAddr, readByteCount, true); // restart, read and send stopbit
    // i2cが実際に読みだしたbyte数だけコピーする
    for (uint8_t i = 0; i < readBytes; i++) {
        regDataPtr[i] = wire.read();
    }
#endif
    return 0;
}

/**
 * @brief 指定されたI2C Slave Deviceのレジスタにデータを書き込みます
 * 
 * @param wire 対象のI2C Peripheral
 * @param slaveAddr 対象のSlave Address
 * @param regAddr 書き込みたい先頭のアドレス
 * @param regData 書き込みたいデータ
 * @return uint8_t Error Code。正常に完了した場合はI2C_ERROR_OK(0)が返ります
 */
uint8_t i2cWriteReg(TwoWire &wire, uint8_t slaveAddr, uint8_t regAddr, const uint8_t* regData, uint16_t writeByteCount)
{
    wire.beginTransmission(slaveAddr);
    wire.write(regAddr);
    for (uint16_t i = 0; i < writeByteCount; i++) {
        wire.write(regData[i]);
    }
    const uint8_t errorCode = Wire.endTransmission(true); // send stopbit
    return errorCode;
}

////////////////////////////////


#include <SPI.h>
#include <Wire.h>

#include "TFT_eSPI/TFT_eSPI.h"
#include "BME680_driver/bme680.h"
#include "Grove_Digital_Light_Sensor/Digital_Light_TSL2561.h"

static TwoWire &wireL = Wire; // left port
// static TwoWire& wireR = Wire1;    // right port

static TFT_eSPI tft = TFT_eSPI();                   // LCD
static TSL2561_CalculateLux &lightSensor = TSL2561; // TSL2561 Digital Light Sensor
struct bme680_dev gasSensor;                        // BME680


// for BME680
void bme680DelayMs(uint32_t period) {
    delay(period);
}
int8_t bme680I2cRead(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len) {
    return i2cReadReg(wireL, dev_id, reg_addr, reg_data, len);
}
int8_t bme680I2CWrite(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len) {
    return i2cWriteReg(wireL, dev_id, reg_addr, reg_data, len);
}


void setup() {
    delay(1000); // por時間を考慮しとく

    Serial.begin(9600);
    // while(!Serial) {}

    // LCD config
    tft.init();
    tft.begin();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);

    // sensor config
    wireL.begin();
    gasSensor.dev_id = BME680_I2C_ADDR_PRIMARY;
    gasSensor.intf = BME680_I2C_INTF;
    gasSensor.read = bme680I2cRead;
    gasSensor.write = bme680I2CWrite;
    gasSensor.delay_ms = bme680DelayMs;

    const int8_t result = bme680_init(&gasSensor);
    tft.print("sensor init=");
    tft.println(result);
    
    // lightSensor.init();

    // startup

    // clear display
    // tft.fillScreen(TFT_WHITE);
    // tft.setCursor(0, 0);
}

void loop() {
    // int32_t visibleLux = lightSensor.readVisibleLux();

    // Serial.print(visibleLux);
    // Serial.println(",");

    delay(100);
}
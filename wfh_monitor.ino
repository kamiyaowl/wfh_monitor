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
uint8_t i2cReadReg(TwoWire& wire, uint8_t slaveAddr, uint8_t regAddr, uint8_t* regDataPtr, uint8_t readByteCount) {
    // readByteCount=0の場合、通信する必要がない
    if (readByteCount == 0) {
        return 0;
    }

    for (uint8_t i = 0; i < readByteCount; i++) {
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
uint8_t i2cWriteReg(TwoWire& wire, uint8_t slaveAddr, uint8_t regAddr, uint8_t regData) {
    wire.beginTransmission(slaveAddr);
    wire.write(regAddr);
    Wire.write(regData);
    const uint8_t errorCode = Wire.endTransmission(true); // send stopbit
    return errorCode;
}

////////////////////////////////
// TODO: グルーロジック以外はライブラリとして分離

#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip
#include <SPI.h>
#include <Wire.h>

#include  "BME680_driver/bme680.h"

static TFT_eSPI tft = TFT_eSPI();
static TwoWire& wire = Wire;
void setup() {
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

    wire.begin();

    // startup
    delay(1000); // por時間を考慮しとく

    // clear display
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
}


void loop() {


    delay(100);
}
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

static TFT_eSPI tft = TFT_eSPI();
static TwoWire& wire = Wire;
const uint8_t I2C_SLAVE_ADDR_MS4 = 0x2a;

/**
 * @brief MS4をリセットします
 */
void ms4Reset(TwoWire& wire) {
    tft.print("MS4 Reset");

    const uint8_t errorCode = i2cWriteReg(wire, I2C_SLAVE_ADDR_MS4, 0xf0, 0xa5); // 0xfcに0xa5を書くとリセット
    if (errorCode != 0) {
        tft.setTextColor(TFT_RED);
        tft.print("Error: ");
        tft.print(errorCode);
        while(true) {}
    }
    tft.print("...");
    delay(10000);
    tft.println("Done");
}

/**
 * @brief FW Versionを読み出して表示します
 */
void ms4ReadFwVersion(TwoWire& wire) {
    tft.print("Read FW Ver");

    uint8_t buf[3] = {};
    // Firmware version, Firmware sub-version, Optional Sensors
    const uint8_t errorCode = i2cReadReg(wire, I2C_SLAVE_ADDR_MS4, 0x80, buf, 3);
    if (errorCode != 0) {
        tft.setTextColor(TFT_RED);
        tft.print(" Error: ");
        tft.print(errorCode);
        while(true) {}
    }

    tft.print(": ");
    tft.print(buf[0], HEX);
    tft.print(".");
    tft.println(buf[1], HEX);
    tft.print("opt sensor MIC:");
    tft.print((buf[2] >> 0x04) & 0x1, HEX);
    tft.print(" CO2:");
    tft.println(buf[2] & 0x1, HEX);
}

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
    ms4ReadFwVersion(wire);
    delay(1000);

    // clear display
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
}


static int textColor = TFT_GREEN;
void loop() {
    // startScan
    // 0xc0 SCAN_START_BYTE: { Reserved, GAS, BATT, AUD, LIGHT, HUM, TEMP, STATUS }
    const uint8_t scanError = i2cWriteReg(wire, I2C_SLAVE_ADDR_MS4, 0xc0, 0x7f);
    if (scanError != 0) {
        tft.setTextColor(TFT_RED);
        tft.print("ScanStart Error: ");
        tft.print(scanError);
        while(true) {}
    }
    delay(100);

    // read DataRegs
    uint8_t readBuf[15] = {};
    const uint8_t readError = i2cReadReg(wire, I2C_SLAVE_ADDR_MS4, 0x00, readBuf, 15);
    if (readError != 0) {
        tft.setTextColor(TFT_RED);
        tft.print("DataRead Error: ");
        tft.print(readError);
        while(true) {}
    }

    const uint8_t  status      = readBuf[0];
    const uint16_t temperature = (readBuf[0x1] << 8) | readBuf[0x2];
    const uint16_t humidity    = (readBuf[0x3] << 8) | readBuf[0x4];
    const uint16_t light       = (readBuf[0x5] << 8) | readBuf[0x6];
    const uint16_t audio       = (readBuf[0x7] << 8) | readBuf[0x8];
    const uint16_t battery     = (readBuf[0x9] << 8) | readBuf[0xa];
    const uint16_t co2         = (readBuf[0xb] << 8) | readBuf[0xc];
    const uint16_t voc         = (readBuf[0xd] << 8) | readBuf[0xe];

    const float temperatureVal = (temperature / 10.0f);
    const float humidityVal    = (humidity / 10.0f);
    const float batteryVal     = (battery / 1024.0f) * (3.3f / 0.330f);

    // 適当に表示しておく
    textColor = (textColor == TFT_GREEN) ? TFT_BLUE : TFT_GREEN;
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(textColor);

    tft.print("temp:");
    tft.println(temperatureVal);
    tft.print("humi:");
    tft.println(humidityVal);
    tft.print("light:");
    tft.println(light);
    tft.print("audio:");
    tft.println(audio);
    tft.print("battery:");
    tft.println(batteryVal);
    tft.print("co2:");
    tft.println(co2);
    tft.print("voc:");
    tft.println(voc);

    delay(100);
}
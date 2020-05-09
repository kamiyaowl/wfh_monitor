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

#include <M5StickC.h>
const uint8_t I2C_SLAVE_ADDR_MS4 = 0x2a;

/**
 * @brief MS4をリセットします
 */
void ms4Reset(TwoWire& wire) {
    M5.Lcd.print("MS4 Reset");

    const uint8_t errorCode = i2cWriteReg(wire, I2C_SLAVE_ADDR_MS4, 0xf0, 0xa5); // 0xfcに0xa5を書くとリセット
    if (errorCode != 0) {
        M5.Lcd.setTextColor(RED);
        M5.Lcd.print("Error: ");
        M5.Lcd.print(errorCode);
        while(true) {}
    }
    M5.Lcd.print("...");
    delay(10000);
    M5.Lcd.println("Done");
}

/**
 * @brief FW Versionを読み出して表示します
 */
void ms4ReadFwVersion(TwoWire& wire) {
    M5.Lcd.print("Read FW Ver");

    uint8_t buf[3] = {};
    // Firmware version, Firmware sub-version, Optional Sensors
    const uint8_t errorCode = i2cReadReg(wire, I2C_SLAVE_ADDR_MS4, 0x80, buf, 3);
    if (errorCode != 0) {
        M5.Lcd.setTextColor(RED);
        M5.Lcd.print(" Error: ");
        M5.Lcd.print(errorCode);
        while(true) {}
    }

    M5.Lcd.print(": ");
    M5.Lcd.print(buf[0], HEX);
    M5.Lcd.print(".");
    M5.Lcd.println(buf[1], HEX);
    M5.Lcd.print("opt sensor MIC:");
    M5.Lcd.print((buf[2] >> 0x04) & 0x1, HEX);
    M5.Lcd.print(" CO2:");
    M5.Lcd.println(buf[2] & 0x1, HEX);
}

void ms4StartScan(TwoWire& wire) {
    M5.Lcd.print("Start Scan...");
    // { Reserved, GAS, BATT, AUD, LIGHT, HUM, TEMP, STATUS }
    uint8_t errorCode = i2cWriteReg(wire, I2C_SLAVE_ADDR_MS4, 0xc0, 0x7f);
    if (errorCode != 0) {
        M5.Lcd.setTextColor(RED);
        M5.Lcd.print(" Error: ");
        M5.Lcd.print(errorCode);
        while(true) {}
    }
    M5.Lcd.println(" Done");
    M5.Lcd.print("Start Verify...");

    uint8_t data = 0x0;
    errorCode = i2cReadReg(wire, I2C_SLAVE_ADDR_MS4, 0xc3, &data, 1);
    if ((errorCode != 0) || (data != 0x7f)) {
        M5.Lcd.setTextColor(RED);
        M5.Lcd.print("\nError code:");
        M5.Lcd.print(errorCode);
        M5.Lcd.print(" data: ");
        M5.Lcd.print(data);
        while(true) {}
    }
}

void setup() {
    // LCD config
    M5.Lcd.begin();
    M5.Lcd.setRotation(3);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);

    Wire.begin();
    Wire.setClock(100000); // 100kHz


    // startup
    ms4ReadFwVersion(Wire);
    ms4StartScan(Wire);

    // clear display
    M5.Lcd.setRotation(0);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
}


static int textColor = GREEN;
void loop() {
    uint8_t readBuf[16] = {};
    const uint8_t  errorCode = i2cReadReg(Wire, I2C_SLAVE_ADDR_MS4, 0x00, readBuf, 16);
    if (errorCode != 0) {
        M5.Lcd.setTextColor(RED);
        M5.Lcd.print("Error: ");
        M5.Lcd.print(errorCode);
        while(true) {}
    }

    const uint8_t  status      = readBuf[0];
    const uint16_t temperature = (readBuf[0x1] << 8) & readBuf[0x2];
    const uint16_t humidity    = (readBuf[0x3] << 8) & readBuf[0x4];
    const uint16_t light       = (readBuf[0x5] << 8) & readBuf[0x6];
    const uint16_t audio       = (readBuf[0x7] << 8) & readBuf[0x8];
    const uint16_t battery     = (readBuf[0x9] << 8) & readBuf[0xa];
    const uint16_t co2         = (readBuf[0xb] << 8) & readBuf[0xc];
    const uint16_t voc         = (readBuf[0xd] << 8) & readBuf[0xe];

    const float temperatureVal = (temperature / 10.0f);
    const float humidityVal    = (humidity / 10.0f);
    const float batteryVal     = (battery / 1024.0f) * (3.3f / 0.330f);

    // 適当に表示しておく
    textColor = (textColor == GREEN) ? ORANGE : GREEN;
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextColor(textColor);

    M5.Lcd.print("temp:");
    M5.Lcd.println(temperature);
    M5.Lcd.print("humidity:");
    M5.Lcd.println(humidity);
    M5.Lcd.print("light:");
    M5.Lcd.println(light);
    M5.Lcd.print("audio:");
    M5.Lcd.println(audio);
    M5.Lcd.print("battery:");
    M5.Lcd.println(battery);
    M5.Lcd.print("co2:");
    M5.Lcd.println(co2);
    M5.Lcd.print("voc:");
    M5.Lcd.println(voc);
}
#include <M5StickC.h>
#include <Wire.h>

const uint8_t I2C_SLAVE_ADDR_MS4 = 0x2a;
/**
 * @brief 指定されたI2C Slave Driveのレジスタからデータを読み出します
 * 
 * @param slaveAddr 対象のSlave Address
 * @param regAddr 読み出したい先頭のアドレス
 * @param regDataPtr 読みだしたデータの格納先のポインタ
 * @param readByteCount 読みだしbyte数
 * @return uint8_t Error Code。正常に完了した場合はI2C_ERROR_OK(0)が返ります
 * 
 * @note readAddrのオートインクリメントにデバイスが対応している必要があります
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


void setup() {
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

    // communication test
    uint8_t buf[3] = {};
    // Firmware version, Firmware sub-version, Optional Sensors
    const uint8_t errorCode = i2cReadReg(Wire, I2C_SLAVE_ADDR_MS4, 0x80, buf, 3);
    if (errorCode != 0) {
        M5.Lcd.setTextColor(RED);
        M5.Lcd.print("I2C Error. code:");
        M5.Lcd.print(errorCode);
        while(true) {}
    }

    M5.Lcd.print("MS4 Firmware version:");
    M5.Lcd.print(buf[0], HEX);
    M5.Lcd.print(".");
    M5.Lcd.print(buf[1], HEX);
}

void loop(){
    uint8_t readBuf[16] = {};
}
#include "IpcQueueDefs.h"
#include "GroveTask.h"

#ifdef WFH_MONITOR_ENABLE_DEBUG
/**
 * @brief Arduinoのシリアルプロッタ用にセンサの値を出力します
 * 
 * @param serial Serial Peripheral
 * @param data 測定したSensor Data
 */
static void debugSerialPrint(Serial_& serial, const MeasureData_t& data) {
    serial.print(data.visibleLux);
    serial.print(",");
    serial.print(data.tempature);
    serial.print(",");
    serial.print(data.pressure);
    serial.print(",");
    serial.print(data.humidity);
    serial.print(",");
    serial.print(data.gas);
    serial.println(",");
}
#endif /* WFH_MONITOR_ENABLE_DEBUG */


void GroveTask::setup(void) {
    this->serial.println("GroveTask setup");
    this->lightSensor.init();
    this->bme680.init();
}

bool GroveTask::loop(void) {
    this->serial.println("GroveTask loop");

    // get sensor datas
    bme680.read_sensor_data();
    const MeasureData_t data = {
        .visibleLux = lightSensor.readVisibleLux(),
        .tempature  = bme680.sensor_result_value.temperature,
        .pressure   = bme680.sensor_result_value.pressure / 1000.0f,
        .humidity   = bme680.sensor_result_value.humidity,
        .gas        = bme680.sensor_result_value.gas / 1000.0f,
        .timestamp  = this->timestamp,
    };
    // TODO: 移動平均を取っておく, StackSizeに注意

    //TODO: Queueに送信

    /* for debug */
    this->timestamp++;

#ifdef WFH_MONITOR_ENABLE_DEBUG
    debugSerialPrint(this->serial, data);
#endif

    return false; /**< no abort */
}

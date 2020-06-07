#include "SysTimer.h"
#include "GroveTask.h"

/**
 * @brief センサの値を出力します
 * 
 * @tparam T print, printlnが使えるclass
 * @param serial Serial Peripheral/ File Handle
 * @param data 測定したSensor Data
 * @param isPrintTimestamp Timestampを出力するか
 */
template<typename T>
static void printData(T& oStream, const MeasureData& data, bool isPrintTimestamp) {
    oStream.print(data.visibleLux);
    oStream.print(",");
    oStream.print(data.tempature);
    oStream.print(",");
    oStream.print(data.pressure);
    oStream.print(",");
    oStream.print(data.humidity);
    oStream.print(",");
    oStream.print(data.gas);
    if (isPrintTimestamp) {
        oStream.print(",");
        oStream.print(data.timestamp);
    }
    oStream.println(",");
}


void GroveTask::setup(void) {
    // configure
    this->resource.config.operate([&](GlobalConfig<FixedConfig::ConfigAllocateSize>& config){
        // fps
        auto fps = GlobalConfigDefaultValues::GroveTaskFps;
        config.read(GlobalConfigKeys::GroveTaskFps, fps);
        this->setFps(fps);
        // debug print
        this->isPrintSerial = GlobalConfigDefaultValues::GroveTaskPrintSerial;
        this->isPrintFile = GlobalConfigDefaultValues::GroveTaskPrintFile;
        config.read(GlobalConfigKeys::GroveTaskPrintSerial, this->isPrintSerial);
        config.read(GlobalConfigKeys::GroveTaskPrintFile, this->isPrintFile);
    });

    // initialize sensor
    // I2C Deviceで問題があったときにsetupでハングアップしないようにタスク内で初期化する
    this->lightSensor.init();
    this->bme680.init();
}

bool GroveTask::loop(void) {
    // Queueに空きがない場合、処理しても仕方ないので抜ける
    if (this->sendQueue.emptyNum() == 0) {
        return false; /**< no abort */            
    }

    // get sensor datas
    bme680.read_sensor_data();
    const MeasureData data = {
        .visibleLux = lightSensor.readVisibleLux(),
        .tempature  = bme680.sensor_result_value.temperature,
        .pressure   = bme680.sensor_result_value.pressure / 100.0f,
        .humidity   = bme680.sensor_result_value.humidity,
        .gas        = bme680.sensor_result_value.gas / 1000.0f,
        .timestamp  = SysTimer::getTickCount(),
    };
    this->sendQueue.send(&data);

    // debug print
    if (this->isPrintSerial) {
        this->resource.serial.operateCritial([&](Serial_& serial){
            printData(serial, data, false);
        });
    }
    if (this->isPrintFile) {
        this->resource.sd.operateCritial([&](SDFS& sd){
            File f = sd.open(FixedConfig::GroveTaskPrintFilePath, FILE_APPEND);
            // 開けなければ失敗
            if (!f) return;
            // 1行追記
            printData(f, data, true);
            // 終わり
            f.close();
        });
    }

    return false; /**< no abort */
}

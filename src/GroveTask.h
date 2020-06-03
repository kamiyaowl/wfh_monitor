#ifndef GROVETASK_H
#define GROVETASK_H

#include <Digital_Light_TSL2561.h>
#include <seeed_bme680.h>

#include "IpcQueueDefs.h"
#include "IpcQueue.h"

#include "FpsControlTask.h"

/**
 * @brief Grove端子に接続されたIICセンサの値を収集するTaskです
 */
class GroveTask : public FpsControlTask {
    public:
        /**
         * @brief Construct a new Grove Task object
         * 
         * @param sendQueue センサー測定値の送信Queue
         * @param serial UARTペリフェラル
         * @param lightSensor 照度センサー。初期化はTask内で行う
         * @param bme680 温湿度、ガスセンサ。初期化はTask内で行う
         */
        GroveTask(IpcQueue<MeasureData>& sendQueue, Serial_& serial, TSL2561_CalculateLux& lightSensor, Seeed_BME680& bme680): sendQueue(sendQueue), serial(serial), lightSensor(lightSensor), bme680(bme680) {}

        /**
         * @brief Destroy the Grove Task object
         */
        virtual ~GroveTask(void) {}
        const char* getName(void) override { return "GroveTask"; }
    protected:
        IpcQueue<MeasureData>& sendQueue; /**< 測定データの送信先 */

        Serial_& serial; /**< for debug */
        TSL2561_CalculateLux& lightSensor;
        Seeed_BME680& bme680;
        void setup(void) override;
        bool loop(void) override;
};

#endif /* GROVETASK_H */
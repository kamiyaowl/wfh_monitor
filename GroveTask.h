#ifndef GROVETASK_H
#define GROVETASK_H

#include <Digital_Light_TSL2561.h>
#include <seeed_bme680.h>

#include "IpcQueueDefs.h"
#include "IpcQueue.h"

#include "TaskBase.h"

/**
 * @brief Grove端子に接続されたIICセンサの値を収集するTaskです
 */
class GroveTask : public TaskBase {
    public:
        GroveTask(IpcQueue<MeasureData_t>& sendQueue, Serial_& serial, TSL2561_CalculateLux& lightSensor, Seeed_BME680& bme680): timestamp(0), sendQueue(sendQueue), serial(serial), lightSensor(lightSensor), bme680(bme680) {}
        virtual ~GroveTask(void) {}
        const char* getName(void) override { return "GroveTask"; }
    private:
        IpcQueue<MeasureData_t>& sendQueue; /**< 測定データの送信先 */

        uint32_t timestamp; /**< for debug */
        Serial_& serial; /**< for debug */
        TSL2561_CalculateLux& lightSensor;
        Seeed_BME680& bme680;
        void setup(void) override;
        bool loop(void) override;
};

#endif /* GROVETASK_H */
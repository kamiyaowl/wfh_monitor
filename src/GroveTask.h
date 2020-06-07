#ifndef GROVETASK_H
#define GROVETASK_H

#include <Digital_Light_TSL2561.h>
#include <seeed_bme680.h>

#include "SharedResourceDefs.h"
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
         * @param resource 共有リソース群
         * @param sendQueue センサー測定値の送信Queue
         * @param lightSensor 照度センサー。初期化はTask内で行う
         * @param bme680 温湿度、ガスセンサ。初期化はTask内で行う
         */
        GroveTask(
            const SharedResourceDefs& resource,
            IpcQueue<MeasureData>& sendQueue,
            TSL2561_CalculateLux& lightSensor,
             Seeed_BME680& bme680
        ): resource(resource), sendQueue(sendQueue), lightSensor(lightSensor), bme680(bme680) {}

        /**
         * @brief Destroy the Grove Task object
         */
        virtual ~GroveTask(void) {}
        const char* getName(void) override { return "GroveTask"; }
    protected:
        const SharedResourceDefs& resource; /**< 共有リソース群 */
        IpcQueue<MeasureData>& sendQueue; /**< 測定データの送信先 */

        TSL2561_CalculateLux& lightSensor;
        Seeed_BME680& bme680;

        bool isPrintSerial; /**< センサ取得値をSerial出力 */
        bool isPrintFile; /**< センサ取得値をSD Card出力 */

        void setup(void) override;
        bool loop(void) override;
};

#endif /* GROVETASK_H */
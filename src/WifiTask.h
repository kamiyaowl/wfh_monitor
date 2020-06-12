#ifndef WIFITASK_H
#define WIFITASK_H

#include <AtWiFi.h>

#include "SharedResourceDefs.h"
#include "IpcQueueDefs.h"
#include "IpcQueue.h"

#include "FpsControlTask.h"

class WifiTask : public FpsControlTask {
    public:
        WifiTask(
            const SharedResourceDefs& resource,
            WiFiClass& wifi,
            IpcQueue<WifiTaskRequest> recvQueue,
            IpcQueue<WifiTaskResponse> sendQueue
            ) : resource(resource), wifi(wifi), recvQueue(recvQueue), sendQueue(sendQueue) {}
        virtual ~WifiTask() {}
        const char* getName(void) override { return "WifiTask"; }
    protected:
        const SharedResourceDefs& resource; /**< 共有リソース */
        WiFiClass& wifi; /**< Wifiを取り扱うのはこのクラスに一任するのでSharedResouceにはしない */
        IpcQueue<WifiTaskRequest> recvQueue; /**< WifiTaskへの要求が積まれるQueue */
        IpcQueue<WifiTaskResponse> sendQueue; /**< WifiTaskからの応答が積まれるQueue */

        void setup(void) override;
        bool loop(void) override;

        bool invokeNop(const WifiTaskRequest& req, WifiTaskResponse& resp);
        bool invokeGetWifiStatus(const WifiTaskRequest& req, WifiTaskResponse& resp);
        bool invokeSend(const WifiTaskRequest& req, WifiTaskResponse& resp);
};

#endif /* WIFITASK_H */
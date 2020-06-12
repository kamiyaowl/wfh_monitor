#ifndef WIFITASK_H
#define WIFITASK_H

#include <AtWiFi.h>

#include "../SharedResourceDefs.h"
#include "../IpcQueueDefs.h"
#include "../IpcQueue.h"
#include "../FpsControlTask.h"

class WifiTask : public FpsControlTask {
    public:
        /**
         * @brief Construct a new Wifi Task object
         * 
         * @param resource 共有リソース群
         * @param wifi Wifiインスタンス
         * @param recvQueue Wifi要求の受信Queue
         * @param sendQueue Wifi応答の送信Queue
         */
        WifiTask(
            const SharedResourceDefs& resource,
            WiFiClass& wifi,
            IpcQueue<WifiTaskRequest>& recvQueue,
            IpcQueue<WifiTaskResponse>& sendQueue
            ) : resource(resource), wifi(wifi), recvQueue(recvQueue), sendQueue(sendQueue) {}
        /**
         * @brief Destroy the Wifi Task object
         */
        virtual ~WifiTask() {}
        const char* getName(void) override { return "WifiTask"; }
    protected:
        const SharedResourceDefs& resource; /**< 共有リソース */
        WiFiClass& wifi; /**< Wifiを取り扱うのはこのクラスに一任するのでSharedResouceにはしない */
        IpcQueue<WifiTaskRequest>& recvQueue; /**< WifiTaskへの要求が積まれるQueue */
        IpcQueue<WifiTaskResponse>& sendQueue; /**< WifiTaskからの応答が積まれるQueue */

        void setup(void) override;
        bool loop(void) override;

        /**
         * @brief NOPが要求されたときの処理
         * 
         * @param req 要求メッセージ
         * @param resp 応答メッセージ
         * @return true 処理は成功
         * @return false 処理は失敗
         */
        bool invokeNop(const WifiTaskRequest& req, WifiTaskResponse& resp);

        /**
         * @brief Wifiのステータスを応答します
         * 
         * @param req 要求メッセージ
         * @param resp 応答メッセージ
         * @return true 処理は成功
         * @return false 処理は失敗
         */
        bool invokeGetWifiStatus(const WifiTaskRequest& req, WifiTaskResponse& resp);

        /**
         * @brief SensorDataをNetwork上に送信します
         * @note 現在はAmbientへの送信を実装
         * 
         * @param req 要求メッセージ
         * @param resp 応答メッセージ
         * @return true 処理は成功
         * @return false 処理は失敗
         */
        bool invokeSend(const WifiTaskRequest& req, WifiTaskResponse& resp);
};

#endif /* WIFITASK_H */
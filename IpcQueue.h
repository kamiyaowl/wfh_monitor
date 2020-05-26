#ifndef IPCQUEUE_H
#define IPCQUEUE_H

#include <cstdint>

#include <Seeed_Arduino_FreeRTOS.h>

/**
 * @brief Task間通信を行うQueueを提供します
 * 
 * @tparam T 送受信するデータ型
 */
template<typename T>
class IpcQueue {
    public:
        /**
         * @brief Construct a new Ipc Queue object
         */
        IpcQueue(void): isInitialized(false), entrySize(sizeof(T)), depth(0) {}
        /**
         * @brief Destroy the Ipc Queue object
         */
        virtual ~IpcQueue(void) {}

        /**
         * @brief Create a RTOS Queue
         * 
         * @param queueDepth Queueの要素数
         * @return true 作成成功
         * @return false 作成失敗
         */
        bool createQueue(size_t queueDepth) {
            // invalid operation
            if (queueDepth == 0) return false;
            // already created
            if (this->isInitialized) return false;

            // create from rtos api
            this->queueHandle = xQueueCreate(queueDepth, this->entrySize);
            if (this->queueHandle == nullptr) return false;

            this->depth = queueDepth;
            this->isInitialized = true;
            return true;
        }

        /**
         * @brief Delete a RTOS Queue
         * 
         * @return true 削除最高
         * @return false 削除失敗
         */
        bool deleteQueue(void) {
            // not created
            if (!this->isInitialized) return true;
            
            vQueueDelete(this->queueHandle);

            this->depth = 0;
            this->isInitialized = false;
        }

        /**
         * @brief Queueの内容をすべてクリアします
         * 
         * @return true クリア成功
         * @return false クリア失敗
         */
        bool reset(void) {
            // not created
            if (!this->isInitialized) return true;

            const auto result = xQueueReset(this->queueHandle);
            return (result == pdTRUE);
        }

        /**
         * @brief Queueにデータを送信します
         * 
         * @param value 送信するデータ
         * @return true 送信成功
         * @return false 送信失敗。未初期化及びQueueu Fullの可能性があります
         */
        bool send(T value) {
            // not created
            if (!this->isInitialized) return false;

            const auto result = xQueueSend(this->queueHandle, value, 0);
            return (result == pdPASS);
        }

        /**
         * @brief Queueからデータを受信します。
         * 
         * @param value 受信するデータの格納先
         * @param isBlocking 受信できるまで待機する場合はtrue
         * @return true 受信成功
         * @return false 受信失敗、未初期化及びQueue Empty、valueがnullptrの可能性があります
         */
        bool receive(T* value, bool isBlocking) {
            // not created
            if (!this->isInitialized) return false;
            // invalid dst ptr
            if (value == nullptr) return false;

            const auto result = xQueueReceive(this->queueHandle, value, (isBlocking ? portMAX_DELAY : 0)); // portMAX_DELAYを指定するとMessageが貯まるまで待つ
            return (result == pdPASS);
        }

        /**
         * @brief Queueに追加された要素数を取得します
         * 
         * @return size_t 要素数
         */
        size_t remainNum(void) {
            // not created
            if (!this->isInitialized) return 0;

            return uxQueueMessagesWaiting(this->queueHandle);
        }

        /**
         * @brief Queueの空き要素数を取得します
         * 
         * @return size_t 空き要素数
         */
        size_t emptyNum(void) {
            // not created
            if (!this->isInitialized) return 0;

            return this->depth - uxQueueMessagesWaiting(this->queueHandle);
        }

        /**
         * @brief Queueに入れることのできる要素数を取得します
         * 
         * @return size_t 最大要素数
         */
        size_t getDepth(void) { 
            return this->depth; 
        }
    protected:
        QueueHandle_t queueHandle;
        bool isInitialized;
        size_t entrySize;
        size_t depth;
};

#endif /* IPCQUEUE_H */
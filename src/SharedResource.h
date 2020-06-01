#ifndef SHAREDRESOURCE_H
#define SHAREDRESOURCE_H

#include <cstdint>

#include <Seeed_Arduino_FreeRTOS.h>

/**
 * @brief Task間共有リソースを定義します
 * @note IpcQueue.h同様 CPU DataCacheの影響を考慮した配置を行ってください
 * 
 * @tparam T 共有リソースの型
 */
template<typename T>
class SharedResource {
    public:
        /**
         * @brief Construct a new Shared Resource object
         * 
         * @param v 管理するデータ。NonCache 属性またはWriteBackが保証される領域に配置されることが望ましいです
         */
        SharedResource(T& v) : value(v) {
            this->semaphoreHandle = xSemaphoreCreateMutex();
        }

        /**
         * @brief Destroy the Shared Resource object
         */
        virtual ~SharedResource(void) {}

        /**
         * @brief リソースロックした上でデータを操作します。セマフォを獲得するまでTaskは待機されます
         * 
         * @tparam FUNC T&を受け取れる関数
         * @param functor 処理関数
         */
        template<class FUNC>
        void operate(FUNC functor) {
            xSemaphoreTake(this->semaphoreHandle, portMAX_DELAY);
            {
                functor(this->value);
            }
            xSemaphoreGive(this->semaphoreHandle);
        }

        /**
         * @brief リソースロックだけでなく、クリティカルセクションでデータ操作を行います
         * @note Tにハードウェアリソースに準ずるもの等を割り当てていて、操作を行う場合はoperateではなくこちらを利用します
         * 
         * @tparam FUNC T&を受け取れる関数
         * @param functor 処理関数
         */
        template<class FUNC>
        void operateCritial(FUNC functor) {
            taskENTER_CRITICAL();
            {
                this->operate(functor);
            }
            taskEXIT_CRITICAL();
        }
    protected:
        SemaphoreHandle_t semaphoreHandle;
        T& value;
};

#endif /* SHAREDRESOURCE_H */

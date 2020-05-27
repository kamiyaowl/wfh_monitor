#ifndef SYSTIMER_H
#define SYSTIMER_H

#include <cstdint>

#include <Seeed_Arduino_FreeRTOS.h>

/**
 * @brief 時間取得昨日を提供します
 */
namespace SysTimer {
    /**
     * @brief FreeRTOSのスケジューラ開始以降のSystickを取得します
     * 
     * @return constexpr uint32_t 
     */
    static uint32_t getTickCount(void) {
        return xTaskGetTickCount();
    }

    /**
     * @brief TickCountを秒に変換します
     * 
     * @tparam T cast可能な数値型
     * @param tick TickCount
     * @return constexpr T tickを秒数に換算した値
     */
    template<typename T>
    static T tickToSec(T tick) {
        return tick / static_cast<T>(configTICK_RATE_HZ);
    }

    /**
     * @brief TickCountをミリ秒に変換します
     * 
     * @tparam T cast可能な数値型, uint32かfloatを指定することを推奨します
     * @param tick TickCount
     * @return constexpr T tickを秒数に換算した値
     */
    template<typename T>
    static T tickToMs(T tick) {
        return tick / static_cast<T>(portTICK_RATE_MS);
    }

    /**
     * @brief TickCountをマイクロ秒に変換します
     * 
     * @tparam T cast可能な数値型, uint32かfloatを指定することを推奨します
     * @param tick TickCount
     * @return constexpr T tickを秒数に換算した値
     */
    template<typename T>
    static T tickToUs(T tick) {
        return tick / (static_cast<T>(portTICK_RATE_MS) / static_cast<T>(1000));
    }

}

#endif /* SYSTIMER_H */
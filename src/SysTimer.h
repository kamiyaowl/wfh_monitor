#ifndef SYSTIMER_H
#define SYSTIMER_H

#include <cstdint>

#include <Seeed_Arduino_FreeRTOS.h>

/**
 * @brief 時間取得関連のAPIを提供します
 */
namespace SysTimer {
    /**
     * @brief FreeRTOSのスケジューラ開始以降のSystickを取得します
     * 
     * @return uint32_t 
     */
    static uint32_t getTickCount(void) {
        return xTaskGetTickCount();
    }

    /**
     * @brief 2つの時間差分をOverflow考慮で計算します
     * @remark 1週してもとのTickを追い越した場合の検知はできません
     * 
     * @param startTick 開始地点
     * @param endTick 終了地点
     * @return uint32_t 差分のTick
     */
    static uint32_t diff(uint32_t startTick, uint32_t endTick) {
        return (startTick < endTick) ? (endTick - startTick) // 通常ケース
                                     : (UINT32_MAX - startTick + endTick); // 裏回った場合
    }

    /**
     * @brief TickCountを秒に変換します
     * 
     * @tparam T cast可能な数値型
     * @param tick TickCount
     * @return T tickを秒数に換算した値
     */
    template<typename T>
    static T tickToSec(T tick) {
        return tick / static_cast<T>(configTICK_RATE_HZ);
    }

    /**
     * @brief 秒をTickCountに変換します
     * 
     * @tparam T cast可能な数値型
     * @param sec 秒
     * @return uint32_t tick
     */
    template<typename T>
    static uint32_t secToTick(T sec) {
        return static_cast<T>(sec * configTICK_RATE_HZ);
    }

    /**
     * @brief TickCountをミリ秒に変換します
     * 
     * @tparam T cast可能な数値型, uint32かfloatを指定することを推奨します
     * @param tick TickCount
     * @return T tickを秒数に換算した値
     */
    template<typename T>
    static T tickToMs(T tick) {
        return tick / static_cast<T>(portTICK_RATE_MS);
    }

    /**
     * @brief ミリ秒をTickCountに変換します
     * 
     * @tparam T cast可能な数値型
     * @param ms ミリ秒
     * @return T tick
     */
    template<typename T>
    static uint32_t msToTick(T ms) {
        return static_cast<T>(ms * portTICK_RATE_MS);
    }
}

#endif /* SYSTIMER_H */
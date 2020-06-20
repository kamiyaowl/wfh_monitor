#ifndef DRAWDEFS_H
#define DRAWDEFS_H

#include <cstdint>
#include <algorithm>

/**
 * @brief 色情報を示します
 */
struct Color {
    uint8_t r; /**< red */
    uint8_t g; /**< green */
    uint8_t b; /**< blue */
};

/**
 * @brief 矩形情報を示します
 */
struct Point {
    int32_t x; /**< X座標 */
    int32_t y; /**< Y座標*/
};

/**
 * @brief 矩形情報を示します
 */
struct Rect {
    int32_t x; /**< 左上X座標 */
    int32_t y; /**< 左上Y座標*/
    uint32_t width; /**< 表示幅、xを基準に右方向に伸びます */
    uint32_t height; /**< 表示高さ、yを基準に下方向に伸びます */
};

#endif /* DRAWDEFS_H */
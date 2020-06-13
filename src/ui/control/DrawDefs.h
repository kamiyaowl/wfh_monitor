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
}
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
    unt32_t width; /**< 表示幅、xを基準に右方向に伸びます */
    unt32_t height; /**< 表示高さ、yを基準に下方向に伸びます */

    /**
     * @brief Construct a new Rect object
     * 
     * @param x 左上座標
     * @param y 左上座標
     * @param width 幅
     * @param height 高さ
     */
    Rect(int32_t x, int32_t y, uint32_t width, uint32_t height) : x(x), y(y), width(width), height(height) {}

    /**
     * @brief Construct a new Rect object
     * 
     * @param p1 座標1
     * @param p2 座標2
     * 
     * @note p1,p2の位置関係に成約はない
     */
    Rect(const Point& p1, const Point& p2) {
        const int32_t x1 = std::min(p1.x, p2.x);
        const int32_t y1 = std::min(p1.y, p2.y);
        const int32_t x2 = std::max(p1.x, p2.x);
        const int32_t y2 = std::max(p1.y, p2.y);
        x = x1;
        y = y1;
        width = (x2 - x1);
        height = (y2 - y1);
    }
};

#endif /* DRAWDEFS_H */
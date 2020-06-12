#ifndef MEASUREDATA_H
#define MEASUREDATA_H

#include <cstdint>

/**
 * @brief 測定データ
 */
struct MeasureData {
    float visibleLux;   /**< 明るさセンサの値 */
    float tempature;    /**< 温度センサの値 */
    float pressure;     /**< 気圧センサの値 */
    float humidity;     /**< 湿度センサの値 */
    float gas;          /**< ガスセンサの値 */
    uint32_t timestamp; /**< for debug */
};

#endif /* MEASUREDATA_H */
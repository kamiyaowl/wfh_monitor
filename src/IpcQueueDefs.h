#ifndef IPCQUEUEDEFS_H
#define IPCQUEUEDEFS_H

#include <cstdint>

#include <AtWiFi.h>

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

/**
 * @brief ボタン入力値の定義
 */
enum class ButtonState : uint32_t {
    None  = 0x00000000,
    Up    = 0x00000001,
    Down  = 0x00000002,
    Left  = 0x00000004,
    Right = 0x00000008,
    Press = 0x00000010,
    A     = 0x00000020,
    B     = 0x00000040,
    C     = 0x00000080,
};

/**
 * @brief ボタン入力情報
 * @note bmpの割当はButtonState以下の定義に従う
 */
struct ButtonEventData {
    uint32_t raw;       /**< 現在の入力値 */
    uint32_t debounce;  /**< チャタリング除去済の値 */
    uint32_t push;      /**< debounceの内、release->push変化した値 */
    uint32_t release;   /**< debounceの内、push->release変化した値 */
    uint32_t timestamp;
};

/**
 * @brief WifiTaskRequestId::GetWifiStatus の応答
 */
struct WifiStatusData {
    uint8_t ipAddr[4]; /**< ip address, isConnected=falseならdon't care */
    wl_status_t status; /**< 接続ステータス */
};

/**
 * @brief WifiTaskへの要求種類
 */
enum class WifiTaskRequestId : uint32_t {
    Nop, /**< no operation */
    GetWifiStatus, /**< Wifi接続状況を表示 */
    SendSensorData, /**< Sensorの値をNetwork経由で送信する */
};

/**
 * @brief WifiTaskへの要求
 */
struct WifiTaskRequest {
    WifiTaskRequestId id; /**< 要求種別 */
    union {
        MeasureData measureData; /**< (id=SendSensorData) 測定データ */
    } data; /**< Data Body、領域がもったいないのでRequestIdごとに切り替えて使う */
};

/**
 * @brief WifiTaskからの応答
 */
struct WifiTaskResponse {
    WifiTaskRequestId id; /**< 要求種別 */
    union {
        WifiStatusData wifiStatus; /**< (id=GetWifiStatus) WiFi接続情報 */
    } data; /**< Data Body、領域がもったいないのでRequestIdごとに切り替えて使う */
    bool isSuccess; /**< エラーなく完了していればtrue(詳細なエラーステータスはdata内に格納すること) */
};

#endif /* IPCQUEUEDEFS_H */
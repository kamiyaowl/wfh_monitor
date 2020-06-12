#ifndef WIFITASKDATA_H
#define WIFITASKDATA_H

#include <cstdint>

#include <AtWiFi.h>

/**
 * @brief WifiTaskRequestId::GetWifiStatus の応答
 */
struct WifiStatusData {
    uint8_t ipAddr[4]; /**< ip address, isConnected=falseならdon't care */
    wl_status_t status; /**< 接続ステータス */
    uint32_t timestamp; /**< 更新時のTickTimerの値 */
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

#endif /* WIFITASKDATA_H */
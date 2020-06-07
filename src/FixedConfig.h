#ifndef FIXEDCONFIG_H
#define FIXEDCONFIG_H

#include <cstdint>

/**
 * @brief コンパイル時に設定が必要な定数群です
 */
namespace FixedConfig {
    static constexpr uint8_t  Bme680SlaveAddr          = 0x76;          /**< BME680のSlave Addr */
    static constexpr uint32_t WaitForPorMs             = 1000;          /**< POR後の待機時間 */
    static constexpr uint32_t SerialBaudrate           = 115200;        /**< UART baudrate */
    static constexpr bool     WaitForInitSerial        = false;         /**< USB Serialが準備できるまでセットアップを継続しない */
    static constexpr char*    ConfigPath               = "wfhm.json";   /**< SD Cardのconfig保存先 */
    static constexpr size_t   ConfigAllocateSize       = 1024;          /**< config格納用に使用する領域サイズ(configの内容が大きい場合は要調整) */
    static constexpr uint32_t ErrorLedPinNum           = 13;            /**< RTOSでエラー発生時のLED Pin番号 */
    static constexpr uint32_t ErrorLedState            = 0;             /**< RTOSでエラー発生時のLEDの状態 */
    static constexpr size_t   DefaultQueueSize         = 4;             /**< SensorData/ButtonStateのQueue Size */
    static constexpr size_t   GroveTaskStackSize       = 8192;          /**< GroveTaskのStackSize */
    static constexpr size_t   ButtonTaskStackSize      = 256;           /**< ButtonTaskのStackSize */
    static constexpr size_t   UiTaskStackSize          = 4096;          /**< UiTaskのStackSize */
    static constexpr uint32_t WaitForDebugPrintMs      = 8192;          /**< Task開始直前の待機時間 */
    static constexpr char*    GroveTaskPrintFilePath   = "sensor.csv";  /**< GroveTaskでファイル記録を有効化した場合の保存先 */
    static constexpr size_t   ButtonTaskDebounceNum    = 2;             /**< ButtonTaskで保持する履歴数 */
    static constexpr size_t   UiTaskBrightnessKeyPoint = 4;             /**< 画面自動調光の設定KeyPoint数 */
}

#endif /* FIXEDCONFIG_H */
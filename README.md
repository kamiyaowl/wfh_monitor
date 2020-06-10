[![Build](https://github.com/kamiyaowl/wfh_monitor/workflows/Build/badge.svg)](https://github.com/kamiyaowl/wfh_monitor/actions?query=workflow%3ABuild)
[![Docs](https://github.com/kamiyaowl/wfh_monitor/workflows/Docs/badge.svg)](https://github.com/kamiyaowl/wfh_monitor/actions?query=workflow%3ADocs)
[![Common](https://github.com/kamiyaowl/wfh_monitor/workflows/Common/badge.svg)](https://github.com/kamiyaowl/wfh_monitor/actions?query=workflow%3ACommon)

# Work from Home Monitor

おうち環境モニター


TODO: 写真

## 機能

TODO:

## 必要なもの

* Wio Terminal: ATSAMD51 Core with Realtek RTL8720DN BLE 5.0 & Wi-Fi 2.4G/5G Dev Board
    * https://www.seeedstudio.com/Wio-Terminal-p-4509.html
* Grove - Temperature, Humidity, Pressure and Gas Sensor for Arduino - BME680
    * https://www.seeedstudio.com/Grove-Temperature-Humidity-Pressure-and-Gas-Sensor-for-Arduino-BME680.html
* Grove - Digital Light Sensor - TSL2561
    * https://www.seeedstudio.com/Grove-Digital-Light-Sensor-TSL2561.html
* Grove - I2C Hub (6 Port)
    * https://www.seeedstudio.com/Grove-I2C-Hub-6-Port-p-4349.html

## 接続方法

TODO:

## 書き込み方法


1. [Build - Github Actions](https://github.com/kamiyaowl/wfh_monitor/actions?query=workflow%3ABuild)からビルド済バイナリをダウンロードします。
2. Bootloaderモードで起動します
    * 方法は[Get Started with Wio Terminal - Seeed Wiki](https://wiki.seeedstudio.com/Wio-Terminal-Getting-Started/#getting-started)を参照。
3. PCに接続するとArduinoのストレージデバイスが見えるので、`wfh_monitor.ino.Seeeduino.samd.seeed_wio_terminal.uf2`をドライブ上にコピーします
4. 無事に起動したら完了

## 操作説明

TODO:

## 設定ファイル

SDカード直下に`wfhm.json`というファイルに設定を記述することで、挙動変更やWiFi機能の有効化を行うことができます。

### 設定内容

[GlobalConfig.h](https://github.com/kamiyaowl/wfh_monitor/blob/master/src/GlobalConfig.h) を参照
`wfhm.json`が作成されていないFAT32で初期化されたSDカードを挿入した状態で起動することで、デフォルト設定の雛形が自動作成されます。


### コンパイル時定数

SDカードでは設定できず、コンパイル時定数として埋め込まれる設定も存在します。
詳細は [FixedConfig.h](https://github.com/kamiyaowl/wfh_monitor/blob/master/src/FixedConfig.h) を参照

## 依存ライブラリ

素晴らしいライブラリをありがとうございます。

* [bblanchon/ArduinoJson](https://github.com/bblanchon/ArduinoJson)
* [lovyan03/LovyanGFX](https://github.com/lovyan03/LovyanGFX)
* [Seeed-Studio/Grove_Digital_Light_Sensor](https://github.com/Seeed-Studio/Grove_Digital_Light_Sensor)
* [Seeed-Studio/Seeed_Arduino_atUnified](https://github.com/Seeed-Studio/Seeed_Arduino_atUnified)
* [Seeed-Studio/Seeed_Arduino_atWiFi](https://github.com/Seeed-Studio/Seeed_Arduino_atWiFi)
* [Seeed-Studio/Seeed_Arduino_atWiFiClientSecure](https://github.com/Seeed-Studio/Seeed_Arduino_atWiFiClientSecure)
* [Seeed-Studio/Seeed_Arduino_FreeRTOS](https://github.com/Seeed-Studio/Seeed_Arduino_FreeRTOS)
* [Seeed-Studio/Seeed_Arduino_FS](https://github.com/Seeed-Studio/Seeed_Arduino_FS)
* [Seeed-Studio/Seeed_Arduino_mbedtls](https://github.com/Seeed-Studio/Seeed_Arduino_mbedtls)
* [Seeed-Studio/Seeed_Arduino_SFUD](https://github.com/Seeed-Studio/Seeed_Arduino_SFUD)
* [Seeed-Studio/Seeed_BME680](https://github.com/Seeed-Studio/Seeed_BME680)
* [Seeed-Studio/esp-at-lib](https://github.com/Seeed-Studio/esp-at-lib)

## 開発者向け情報

### ドキュメント

doxygenを整備しています。 [Docs - GitHub Actions](https://github.com/kamiyaowl/wfh_monitor/actions?query=workflow%3ADocs) から最新のものをDLするか、`$ docker-compose run docs`, `$ doxygen`等でビルドすることで入手することができます。

### FWのビルド方法

Dockerを推奨しますが、同様の環境を作れば任意の環境で開発できます。
安定性確保のためには`lib`下のモジュールとバージョンを合わせることを推奨します。

#### Docker

```sh
$ docker-compose run build
```

#### Arduino CLI & Python

`.lib`下にあるライブラリと`Seeeduino:seeed_wio_terminal`の追加が必要です。
詳しくは [arduino-cli.Dockerfile](https://github.com/kamiyaowl/wfh_monitor/blob/master/arduino-cli.Dockerfile) をご参照ください

```sh
$ ./build.sh
```

#### Arduino IDE

1. seeed_wio_terminalをボードマネージャから追加します
2. `./lib`下にあるライブラリをインストールします
3. にwfh_monitor.inoを開いてコンパイルして書き込んでください。

## License

MIT

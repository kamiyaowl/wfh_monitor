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

## 依存ライブラリ

素晴らしいライブラリをありがとうございます。

* [lovyan03/LovyanGFX](https://github.com/lovyan03/LovyanGFX)
* [Seeed-Studio/Grove_Digital_Light_Sensor](https://github.com/Seeed-Studio/Grove_Digital_Light_Sensor)
* [Seeed-Studio/Seeed_Arduino_FreeRTOS](https://github.com/Seeed-Studio/Seeed_Arduino_FreeRTOS)
* [Seeed-Studio/Seeed_BME680](https://github.com/Seeed-Studio/Seeed_BME680)

## 開発者向け情報

### ドキュメント

細かいものは用意できていませんが、doxygenを整備しています。

[Docs - GitHub Actions](https://github.com/kamiyaowl/wfh_monitor/actions?query=workflow%3ADocs) から最新のものをDLするか、`$ docker-compose run docs`, `$ doxygen`のいずれかで入手することができます。

### 自分でビルドする方法

Dockerを推奨しますが、同様の環境を作れば任意の環境で開発できます。
安定性確保のためには`lib`下のモジュールとバージョンを合わせることを推奨します。

#### Docker

```sh
$ docker-compose run build
```

#### Arduino CLI & Python

`.lib`下にあるライブラリ、`Seeeduino:seeed_wio_terminal`の追加が必要です。
詳しくは `arduino-cli.Dockerfile`をご参照ください

```sh
$ ./build.sh
```

#### Arduino IDE

1. seeed_wio_terminalをボードマネージャから追加します
2. `./lib`下にあるライブラリをインストールします
3. にwfh_monitor.inoを開いてコンパイルして書き込んでください。

## License

MIT

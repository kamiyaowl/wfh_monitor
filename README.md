[![Build](https://github.com/kamiyaowl/wfh_monitor/workflows/Build/badge.svg)](https://github.com/kamiyaowl/wfh_monitor/actions?query=workflow%3ABuild)
[![Docs](https://github.com/kamiyaowl/wfh_monitor/workflows/Docs/badge.svg)](https://github.com/kamiyaowl/wfh_monitor/actions?query=workflow%3ADocs)

# Work from Home Monitor

おうち環境モニター

## 必要なもの

* Wio Terminal: ATSAMD51 Core with Realtek RTL8720DN BLE 5.0 & Wi-Fi 2.4G/5G Dev Board
    * https://www.seeedstudio.com/Wio-Terminal-p-4509.html
* Grove - Temperature, Humidity, Pressure and Gas Sensor for Arduino - BME680
    * https://www.seeedstudio.com/Grove-Temperature-Humidity-Pressure-and-Gas-Sensor-for-Arduino-BME680.html
* Grove - Digital Light Sensor - TSL2561
    * https://www.seeedstudio.com/Grove-Digital-Light-Sensor-TSL2561.html
* Grove - I2C Hub (6 Port)
    * https://www.seeedstudio.com/Grove-I2C-Hub-6-Port-p-4349.html

## Build/Upload

[ビルド済バイナリはこちら](https://github.com/kamiyaowl/wfh_monitor/actions?query=workflow%3ABuild)

### Arduino IDE

wfh_monitor.inoを開いてコンパイルして書き込んでください。seeed_wio_terminalが追加されている必要があります。
また`./lib`下にあるライブラリのインストールが必要です。

### Arduino CLI

```sh
# build
$ arduino-cli compile -b Seeeduino:samd:seeed_wio_terminal ./wfh_monitor.ino --verbose --log-level trace

# upload
arduino-cli upload -p COM10 -i .\wfh_monitor.ino.Seeeduino.samd.seeed_wio_terminal.bin -b Seeeduino:samd:seeed_wio_terminal --verbose --log-level trace --additional-urls https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
```

### Docker

```sh
# build
$ docker-compose run build

# upload: docker-compose.ymlで対象デバイスのSerialportを指定
$ docker-compose run upload
```

Windowsではまだうまくアップロードできていないので...。

```
$ ./upload.bat
```

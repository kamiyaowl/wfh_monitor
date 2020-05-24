![Build](https://github.com/kamiyaowl/wfh_monitor/workflows/Build/badge.svg)

# Work from Home Monitor

おうち環境モニター

## Build/Upload

[ビルド済バイナリはこちら](https://github.com/kamiyaowl/wfh_monitor/actions?query=workflow%3ABuild)

### Arduino IDE

wfh_monitor.inoを開いてコンパイルして書き込んでください。seeed_wio_terminalが追加されている必要があります

### Arduino CLI

```sh
# build
$ arduino-cli compile -b Seeeduino:samd:seeed_wio_terminal ./wfh_monitor.ino --verbose --log-level trace

# upload
arduino-cli upload -p COM10 -i .\wfh_monitor.ino.Seeeduino.samd.seeed_wio_terminal.bin -b Seeeduino:samd:seeed_wio_terminal --verbose --log-level trace --additional-urls https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
```

### Docker

```sh
# build container
$ docker build . -t wfh_monitor

# build
$ docker-compose run build

# upload: docker-compose.ymlで対象デバイスのSerialportを指定
$ docker-compose run upload
```

Windowsではまだうまくアップロードできていないので...。

```
$ ./upload.bat
```

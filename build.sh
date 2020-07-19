#!/bin/sh
arduino-cli compile -b Seeeduino:samd:seeed_wio_terminal ./wfh_monitor.ino --output-dir ./ --verbose --log-level trace
python ./utils/uf2/utils/uf2conv.py -c -b 0x4000 -o ./wfh_monitor.ino.uf2 ./wfh_monitor.ino.bin

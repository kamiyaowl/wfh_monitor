FROM ubuntu:20.04

# install from apt package
RUN apt update && apt install -y \
    curl

# install arduino-cli
RUN mkdir -p /tool
RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=/tool sh
ENV PATH $PATH:/tool/
RUN echo $PATH

# refresh platform indexes
RUN arduino-cli core update-index
RUN arduino-cli core update-index --additional-urls https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json

# add arduino core and libraries
RUN arduino-cli core install arduino:samd
RUN arduino-cli core install Seeeduino:samd --additional-urls https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
# RUN arduino-cli core list # for debug

# add 3rd-party library
# RUN  arduino-cli lib install "libname"

# create work directory
RUN mkdir /work
WORKDIR /work

CMD ["/bin/sh"]
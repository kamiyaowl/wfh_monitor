FROM ubuntu:20.04

# install from apt package
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && apt-get install -y curl python

# install arduino-cli
RUN mkdir -p /tool
RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=/tool sh
ENV PATH $PATH:/tool/
RUN echo $PATH

# refresh platform indexes
# ビルドキャッシュされると最新のBoardが見つからなくなる可能性があるので--no-cacheでビルドすること
RUN arduino-cli core update-index
RUN arduino-cli core update-index --additional-urls https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json

# add arduino core and libraries
RUN arduino-cli core install Seeeduino:samd@1.7.6 --additional-urls https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
RUN arduino-cli core list

# create work directory
RUN mkdir /work
WORKDIR /work

CMD ["/bin/sh"]
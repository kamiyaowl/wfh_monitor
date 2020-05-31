FROM ubuntu:20.04

# install from apt package
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && apt-get install -y doxygen graphviz git

# create work directory
RUN mkdir /work
WORKDIR /work

CMD ["/bin/sh"]
FROM ubuntu:20.04

# install from apt package
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && apt-get install -y doxygen graphviz git

# create work directory
RUN mkdir /wfh_monitor
WORKDIR /wfh_monitor

CMD ["/bin/sh"]
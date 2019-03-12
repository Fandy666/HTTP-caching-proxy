FROM ubuntu:18.04
RUN mkdir /var/log/erss
RUN mkdir /code
RUN apt-get update && apt-get -y install build-essential g++ make net-tools
WORKDIR /code
ADD . /code


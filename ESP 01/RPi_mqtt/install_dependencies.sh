#!/bin/bash

sudo apt-get update

sudo apt-get upgrade

sudo apt-get install software-properties-common
sudo add-apt-repository ppa:deadsnakes/ppa
sudo apt-get update
sudo apt-get install -y python3.8 python3-pip

sudo apt-get install -y influxdb influxdb-client

sudo apt-get install -y mosquitto mosquitto-clients

sudo pip3 install paho-mqtt influxdb

sudo apt-get install -y adduser libfontconfig1
wget https://dl.grafana.com/oss/release/grafana_7.3.1_amd64.deb
sudo dpkg -i grafana_7.3.1_amd64.deb

sudo apt-get install grafana -y

sudo apt-get install chromium-bsu -y

sudo mv mosquitto.conf /etc/mosquitto/mosquitto.conf

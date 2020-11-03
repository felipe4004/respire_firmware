#!/bin/bash

echo "Insira o usuario e senha para a configuracao do broker!" 
sudo mosquitto_passwd -c /etc/mosquitto/pwfile

echo "Verificando o funcionamento do broker:"

sudo systemctl status mosquitto

  sudo systemctl enable mosquitto


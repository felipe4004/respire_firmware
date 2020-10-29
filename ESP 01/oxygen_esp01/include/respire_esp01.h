#ifndef RESPIRE_ESP01_H_
#define RESPIRE_ESP01_H_

#include<SoftwareSerial.h>
#include <Arduino.h> 
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <FS.h>


#define EEPROM_tam 512
#define UPDATE_SIZE_UNKNOWN 0xFFFFFFFF


//Strings que armazenaram as informacoes vindas por UART.
String ard_receiver;
String blu_receiver;


bool ard_ok;
bool blu_ok;

//SSID e passowrd do wifi, bem como o host name usado pelo dispositivo.
char ssid[50]= {0};
char password[50]={0};
const char* hostname = "oxygen_0000";


//struct para armazenamento dos status vindos do arduino.
struct serial_info{
  char  press_pico[5];
  char  vol_corr[5];
  char  fr[5];
  char  PEEP[5];
};

//struct para armazenamento das configuraçoes vindas do modulo bluetooth.
struct serial_config{

  char vol_corr[5];
  char fluxo[5];
  char t_fluxo[5];
  char i_e[5];
  char PEEP[5];

};

// Declaração do serial por software
SoftwareSerial espSerial(2,0); //RX/TX via soft

//Web server inicializado sob o prefixo server.
ESP8266WebServer server(80);


void send_blu (struct serial_info *send);
void arduino_send (struct serial_config *ard_send);
void serialEvent();
void espSerialEvent();
void handleForm();
void handleWificredentials();
void loginIndex();
void serverIndex();





#endif
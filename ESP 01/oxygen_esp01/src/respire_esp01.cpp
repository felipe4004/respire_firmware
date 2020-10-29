#include "respire_esp01.h"


//Serial event ocorre sempre que uma mensagem chega via serial
void serialEvent() {

  //se houver bytes disponíves, entao serial.available será 1
  while (Serial.available()) {
    
  //pega o novo byte. Lembrando que UART so envia 1 word por vez
  char inputChar = (char)Serial.read();
  
  //O byte é incrementado numa variavel tipo string até que o comando \n seja verificado
  
    ard_receiver += inputChar;
    if (inputChar == '\n') {
      
      ard_ok = true;
      
    }
  }
}

void espSerialEvent(){
  if (espSerial.available()){
    char inputChar = (char)espSerial.read();

    blu_receiver += inputChar;

    if(inputChar=='\n'){
      blu_ok=true;
    }
  }
}

//=====================
/*Funcoes para envio das variaveis via serial*/
//=====================

void send_blu (struct serial_info *send){
  espSerial.write(send->press_pico);
  espSerial.write(";");
  espSerial.write(send->vol_corr);
  espSerial.write(";");
  espSerial.write(send->fr);
  espSerial.write(";");
  espSerial.write(send->PEEP);
  espSerial.write("\n");
}

void arduino_send (struct serial_config *ard_send){
  Serial.write(ard_send->vol_corr);
  espSerial.write(";");
  Serial.write(ard_send->fluxo);
  espSerial.write(";");
  Serial.write(ard_send->t_fluxo);
  espSerial.write(";");
  Serial.write(ard_send->i_e);
  espSerial.write(";");
  Serial.write(ard_send->PEEP);
  espSerial.write("\n");
}


//===============================================================
// Essa rotina eh executada quando se aperta o botao submit do wifi
//===============================================================
void handleForm() {
  String ssid_wifi = server.arg("SSID_wifi")+";"; 
  String pass = server.arg("pass_wifi") + ";"; 
 
  EEPROM.begin(EEPROM_tam);
  
  size_t n=0;
  
  for(; n<ssid_wifi.length();n++){
    EEPROM.write(n,ssid_wifi[n]);
  }
  for(size_t j=0; j<pass.length();j++, n++){
    EEPROM.write(n, pass[j]);
  }

  EEPROM.commit();
  EEPROM.end();

  ESP.restart();
}

//=====================
/*Funcao para armazenamento das credenciais Wireless na EEPROM*/
//=====================

void handleWificredentials() {
  EEPROM.begin(EEPROM_tam);

  size_t n=0;
  char aux='0';
  while(aux!=';'){
     
     ssid[n]=EEPROM.read(n);
     aux=EEPROM.read(n+1);
     n++;

  }
  
  size_t j=0;
  n++;
  aux='0';  
  while(aux!=';'){
     
     password[j]=EEPROM.read(n);
     aux=EEPROM.read(n+1);
     j++;
     n++;
  
  }
  EEPROM.end();
}


//=====================
/*Funcoes para leitura da SPIFSS e envio dos arquivos html.*/
//=====================

void loginIndex() {
  File loginIndex = SPIFFS.open("/loginIndex", "r");
  server.streamFile(loginIndex, "text/html");
}

void serverIndex() {
  File serverIndex = SPIFFS.open("/serverIndex", "r");
  server.streamFile(serverIndex, "text/html");
}
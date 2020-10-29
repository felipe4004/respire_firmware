#include "respire_esp01.h"



void setup() {
  
  Serial.begin(9600); //abertura da porta de hardware serial.
  espSerial.begin(38400); //abertura da porta serial de software.
  //A taxa de comunicacao padrao para modulos bluetooth eh 38400
  //===============================================================
  //  Configuracao Bluetooth module
  //===============================================================
  delay(2000);
  espSerial.write("AT+NAME=oxygen_0000");
  espSerial.write("AT+PSWD=oxigen_0000");
  espSerial.write("AT+UART=38400");
  
  //Inicializacao do SPIFSS
  SPIFFS.begin();

  //===============================================================
  //  Configuracao WiFi
  //===============================================================
  handleWificredentials();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) 
    {
      delay(2000);
      ESP.restart();
     }

//===============================================================
// Habilitando o mDNS sob o nome http://oxygenXXXX.local/, onde XXXX é o identificador do dispositivo
//===============================================================
    /*Use o mDNS para mapear o hostname do endereco IP*/
      MDNS.begin(hostname);

  //===============================================================
  // Configuracao do webserver que gerencia a atualizacao ota
  //===============================================================
        /*Retorna a pagina de login que é armazenada na string loginindex*/
        server.on("/", loginIndex);

        /*Retorna a pagina index que é armazenada no string serverIndex*/
        server.on("/serverIndex", serverIndex);
        
        /*Manipula a variavel de senha Wifi e armazena-a na EEPROM*/
        server.on("/wifi_manager", handleForm);
        
        /*manipula o envio do arquivo binario para atualizacao OTA*/
        server.on("/update", HTTP_POST, []() {
          server.sendHeader("Connection", "close");
          server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
          ESP.restart();
        }, []() {
          HTTPUpload& upload = server.upload();
          
          if (upload.status == UPLOAD_FILE_START) 
            {
              Serial.printf("Update: %s\n", upload.filename.c_str());
              if (!Update.begin(UPDATE_SIZE_UNKNOWN)) 
                { 
                 Update.printError(Serial);
                }
            } 
          else if (upload.status == UPLOAD_FILE_WRITE) 
            {
              
         /* flashing firmware na ESP*/
              if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) 
                {
                 Update.printError(Serial);
                 }
             } 
          else if (upload.status == UPLOAD_FILE_END) {
               if (Update.end(true)) 
                  { 
                   } 
          else 
            {
              Update.printError(Serial);
            }
            
          }
        });

    delay(2000);
}

void loop() {
  /*Declaracao das structs */
  struct serial_info info;
  struct serial_config config;


  // apenas responde quando dados são recebidos:
  while(ard_ok) {
    /*Tratamento da String para os respectivos valores em info*/
    size_t n=0;
    for(;(ard_receiver[n]=';');n++){
      info.press_pico[n]= ard_receiver[n];
    }
    n++;
    for(size_t j=0; (ard_receiver[n]=';');n++, j++){
      info.vol_corr[j]=ard_receiver[n];
    }
    n++;
    for(size_t j=0; (ard_receiver[n]=';');n++, j++){
      info.fr[j]=ard_receiver[n];
    }
    n++;
    for(size_t j=0; (ard_receiver[n]='\n');n++, j++){
      info.PEEP[j]=ard_receiver[n];
    }
    ard_receiver = "";
    ard_ok = false;
    send_blu(&info);

  }
  
  if (espSerial.available()) {
    espSerialEvent();
    while (blu_ok) {

    size_t n=0;
    for(;(blu_receiver[n]=';');n++){
      config.vol_corr[n]= ard_receiver[n];
    }
    n++;
    for(size_t j=0; (blu_receiver[n]=';');n++, j++){
      config.fluxo[j]=ard_receiver[n];
    }
    n++;
    for(size_t j=0; (blu_receiver[n]=';');n++, j++){
      config.t_fluxo[j]=ard_receiver[n];
    }
    n++;
    for(size_t j=0; (blu_receiver[n]='\n');n++, j++){
      config.i_e[j]=ard_receiver[n];
    }
    for(size_t j=0; (blu_receiver[n]='\n');n++, j++){
      config.PEEP[j]=ard_receiver[n];
    }
    blu_receiver = "";
    blu_ok = false;
    arduino_send(&config);

    }
  }
  
  server.handleClient();


}




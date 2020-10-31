#include<SoftwareSerial.h> 
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <FS.h>


#define EEPROM_tam 512
#define UPDATE_SIZE_UNKNOWN 0xFFFFFFFF


SoftwareSerial espSerial(2,0); //RX/TX via soft

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

//Web server inicializado sob o prefixo server.
ESP8266WebServer server(80);


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

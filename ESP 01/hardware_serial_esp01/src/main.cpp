/************
 * Programador: Felipe Rodrigues Sobrinho
 * Universidade de Brasília
 * 24 de outubro de 2020
 * Respirador mecânico AMBU Respire
 ************/ 


#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>


void wifi_setup();
void mdns_setup();
void handleroot();
void handlenotfound();
void server_initialize();

const char * ssid = "Bifrost";    //O SSID da rede a qual deseja conectar-se
const char * password = "089083jkb139293709b1393401023fe"; //A senha da respectiva rede
unsigned long current_time = 0, previous_time = 100;

// union dados_recebidos{
//   struct{
//   short int PEEP, PEEPmin, PEEPmax;
//   short int volcorr, volcorrmax, volcorrmin;
//   short int fluxo, fluxo_max, fluxo_min;
//   short int id;
//   };
//   short int receive[10] ={1,2,3,4,5,6,7,8,9,10};
// } data;
union dados{

struct {
  char PEEP[4], PEEPmin[4], PEEPmax[4];
  char volcorr[4], volcorrmax[4], volcorrmin[4];
  char fluxo[4], fluxo_max[4], fluxo_min[4];
  char id[4];
  }; 
  char recebido[41];
}data;




ESP8266WebServer server(80);


void setup() {
  Serial.begin(4800);
  delay(10);
  Serial.println("\n");

  wifi_setup();
  mdns_setup();
  server_initialize();

}

void loop() {
  server.handleClient();
  MDNS.update();
  current_time = millis();

  if((current_time- previous_time) >= 2000){
    previous_time = current_time;
    if(Serial.available()>0){
      Serial.readBytes(data.recebido, 40);
    } 
  }
}


void wifi_setup(){
  WiFi.begin(ssid, password);
  Serial.println("Conectando ao ");
  Serial.print(ssid);
  Serial.println("......");

  while(WiFi.status() != WL_CONNECTED) {
    delay(1);
    if(WiFi.status() == WL_CONNECT_FAILED){
      Serial.println("Falha na conexão!\n");
      break;
    }
  }

}

void mdns_setup() {
  if (MDNS.begin("respireid")) {             
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }
  
}

void server_initialize(){
  server.on("/", HTTP_GET, handleroot);
  server.begin();
  MDNS.addService("http", "tcp", 80);
}

void handleroot(){
  char str[1024];
  char aux[50];


  sprintf(str, "<p1>Numeros escritos pela serial:</p1>");
  sprintf(aux, "<p2>PEEP: %s</p2>", data.PEEP);
  strcat(str, aux);
  sprintf(aux, "<p2>PEEPmin: %s</p2>", data.PEEPmin);
  strcat(str, aux);
  sprintf(aux, "<p2>PEEPmax: %s</p2>", data.PEEPmax);
  strcat(str, aux);
  sprintf(aux, "<p2>volcorr: %s</p2>", data.volcorr);
  strcat(str, aux);
  sprintf(aux, "<p2>volcorrmax: %s</p2>", data.volcorrmax);
  strcat(str, aux);
  sprintf(aux, "<p2>volcorrmin: %s</p2>", data.volcorrmin);
  strcat(str, aux);
  sprintf(aux, "<p2>fluxo: %s</p2>", data.fluxo);
  strcat(str, aux);
  sprintf(aux, "<p2>fluxo_max: %s</p2>", data.fluxo_max);
  strcat(str, aux);
  sprintf(aux, "<p2>fluxo_min: %s</p2>", data.fluxo_min);
  strcat(str, aux);
  sprintf(aux, "<p2>id: %s</p2>", data.id);
  strcat(str, aux);
  Serial.println(str);
  server.send(200, "text/html", str);
  Serial.println(Serial.getRxBufferSize());
}

void handlenotfound(){
 server.send(404, "text/plain", "404: Not found");
}


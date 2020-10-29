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

union dados_recebidos{
  struct{
  short int PEEP, PEEPmin, PEEPmax;
  short int volcorr, volcorrmax, volcorrmin;
  short int fluxo, fluxo_max, fluxo_min;
  short int id;
  };
  int receive[5];
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
  size_t i=0;
  server.handleClient();
  MDNS.update();
  current_time = millis();
  if((current_time- previous_time) >= 1000){
    Serial.write(i);
    previous_time = current_time;
  }
  if(Serial.available()>5){
    delayMicroseconds(50);
    for(i = 0; i <= 5; i++)
      data.receive[i] = Serial.read();
    
  }
    

}


void wifi_setup(){
  WiFi.begin(ssid, password);
  Serial.println("Conectando ao ");
  Serial.print(ssid);
  Serial.println("......");

  while(WiFi.status() != WL_CONNECTED) {
    if(WiFi.status() == WL_CONNECT_FAILED){
      Serial.println("Falha na conexão!\n");
      break;
    }
    delay(2);
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
  sprintf(aux, "<p2>PEEP: %hu</p2>", data.PEEP);
  strcat(str, aux);
  sprintf(aux, "<p2>PEEPmin: %hu</p2>", data.PEEPmin);
  strcat(str, aux);
  sprintf(aux, "<p2>PEEPmax: %hu</p2>", data.PEEPmax);
  strcat(str, aux);
  sprintf(aux, "<p2>volcorr: %hu</p2>", data.volcorr);
  strcat(str, aux);
  sprintf(aux, "<p2>volcorrmax: %hu</p2>", data.volcorrmax);
  strcat(str, aux);
  sprintf(aux, "<p2>volcorrmin: %hu</p2>", data.volcorrmin);
  strcat(str, aux);
  sprintf(aux, "<p2>fluxo: %hu</p2>", data.fluxo);
  strcat(str, aux);
  sprintf(aux, "<p2>fluxo_max: %hu</p2>", data.fluxo_max);
  strcat(str, aux);
  sprintf(aux, "<p2>fluxo_min: %hu</p2>", data.fluxo_min);
  strcat(str, aux);
  sprintf(aux, "<p2>id: %hu</p2>", data.id);
  strcat(str, aux);
  
  server.send(200, "text/html", str);
}

void handlenotfound(){
 server.send(404, "text/plain", "404: Not found");
}


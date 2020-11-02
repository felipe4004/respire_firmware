#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h> 
#include <SoftwareSerial.h>

//SoftwareSerial softserial(0,2); // RX, TX // Make pin 2 on Arduino the rduino’s RX line.

// WiFi
const char* ssid = "Bifrost";                 
const char* wifi_password = "089083jkb139293709b1393401023fe"; 

// MQTT
const char* mqtt_server = "192.168.1.5";  // IP do MQTT broker
const char* pressure_topic = "uti/001/pressure";
const char* flow_topic = "uti/001/flow";
const char* mqtt_username = "felipemqtt"; // MQTT username
const char* mqtt_password = "12345"; // MQTT password
const char* clientID = "client_livingroom"; // MQTT client ID
char str[100];
size_t tam_str=0;

//Dados 

struct dados{
  char pres[4], pres_min[4], pres_max[4];
  char volcorr[4], volcorrmax[4], volcorrmin[4];
  char fluxo[4], fluxo_max[4], fluxo_min[4];
  char id[4];
  }data; 


WiFiClient wifiClient;

PubSubClient client(mqtt_server, 1883, wifiClient);

void trans_data();
void connect_MQTT();
void mqtt_publish();



void setup() {
  //Serial.print("Connecting to ");
  //Serial.println(ssid);

  WiFi.begin(ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    // Serial.print(".");
  }

  //Serial.println("WiFi connected");
 // Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());
  Serial.begin(115200);
  //softserial.begin(38400);

  connect_MQTT();

}

void loop() {


  //softserial.setTimeout(2000);

  if(Serial.available()>0){
    tam_str = Serial.readBytesUntil(';',str, 100);
    if(tam_str > 0){
      //softserial.print(str);
      //Serial.println(str);
      trans_data();
      mqtt_publish();


    }
  }
  
  
  //client.disconnect(); 
}

void trans_data(void) {
  if ((str[0] == '>') && (str[tam_str-1] == '<')){
    for(size_t sel=0, j=0, i=1; i<tam_str ; i++){
      switch(sel){
        case(0):
          if (str[i] == ','){
            sel++;
            data.pres[j] = '\0';
            j=0;
          }
          else{
            data.pres[j] = str[i];
            j++;
          }
          break;
        case(1):
          if (str[i] == ','){
            sel++;
            data.pres_min[j] = '\0';
            j=0;
          }
          else{
            data.pres_min[j] = str[i];
            j++;
          }
          break;
        case(2):
          if (str[i] == ','){
            sel++;
            data.pres_max[j] = '\0';
            j=0;
          }
          else{
            data.pres_max[j] = str[i];
            j++;
          }
          break;
        case(3):
          if (str[i] == ','){
            sel++;
            data.volcorr[j] = '\0';
            j=0;
          }
          else{
            data.volcorr[j] = str[i];
            j++;
          }
          break;
        case(4):
          if (str[i] == ','){
            sel++;
            data.volcorrmax[j] = '\0';
            j=0;
          }
          else{
            data.volcorrmax[j] = str[i];
            j++;
          }
          break;
        case(5):
          if (str[i] == ','){
            sel++;
            data.volcorrmin[j] = '\0';
            j=0;
          }
          else{
            data.volcorrmin[j] = str[i];
            j++;
          }
          break;
        case(6):
          if (str[i] == ','){
            sel++;
            data.fluxo[j] = '\0';
            j=0;
          }
          else{
            data.fluxo[j] = str[i];
            j++;
          }
          break;
        case(7):
          if (str[i] == ','){
            sel++;
            data.fluxo_max[j] = '\0';
            j=0;
          }
          else{
            data.fluxo_max[j] = str[i];
            j++;
          }
          break;
        case(8):
          if (str[i] == ','){
            sel++;
            data.fluxo_min[j] = '\0';
            j=0;
          }
          else{
            data.fluxo_min[j] = str[i];
            j++;
          }
          break;
        case(9):
          if (str[i] == '<'){
            sel++;
            data.id[j] = '\0';
            j=0;
          }
          else{
            data.id[j] = str[i];
            j++;
          }
          break;
        default:
          break;
      }
    }
  }

}


void connect_MQTT(){
  
  if (client.connect(clientID, mqtt_username, mqtt_password)) {
    //Serial.println("Connected to MQTT Broker!");
  }
/*   else {
    Serial.println("Connection to MQTT Broker failed...");
  } */

}


void mqtt_publish(){
/*   Serial.print("Pressure: ");
  Serial.print(data.pres);
  Serial.println(" cmH2O");
  Serial.print("Flow: ");
  Serial.print(data.fluxo);
  Serial.println(" L/min");
 */
  if (client.publish(pressure_topic, String(data.pres).c_str())) {
    // Serial.println("Pressao enviada!");
  }
  else {
    //Serial.println("Falha ao enviar pressao. Reconectando ao broker e tentando novamente");
    client.connect(clientID, mqtt_username, mqtt_password);
    client.publish(pressure_topic, String(data.pres).c_str());
  }
  if (client.publish(flow_topic, String(data.fluxo).c_str())) {
    //Serial.println("fluxo enviado!");
  }
  else {
    //Serial.println("Falha ao enviar fluxo. Reconectando ao broker e tentando novamente");
    client.connect(clientID, mqtt_username, mqtt_password);
    client.publish(flow_topic, String(data.fluxo).c_str());
  }

}
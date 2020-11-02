/************
 * Programador: Felipe Rodrigues Sobrinho
 * Universidade de Brasília
 * 24 de outubro de 2020
 * Respirador mecânico AMBU Respire
 ************/ 


#include <Arduino.h>

void handleroot();

char str[100] = {0};
size_t tam_str;

// union dados_recebidos{
//   struct{
//   short int PEEP, PEEPmin, PEEPmax;
//   short int volcorr, volcorrmax, volcorrmin;
//   short int fluxo, fluxo_max, fluxo_min;
//   short int id;
//   };
//   short int receive[10] ={1,2,3,4,5,6,7,8,9,10};
// } data;

struct dados{
  char PEEP[4], PEEPmin[4], PEEPmax[4];
  char volcorr[4], volcorrmax[4], volcorrmin[4];
  char fluxo[4], fluxo_max[4], fluxo_min[4];
  char id[4];
  }data; 



void setup() {
  Serial.begin(115200);
  delay(10);


}

void loop() {
  if(Serial.available()>0){
    tam_str = Serial.readBytes(str, 100);
    if(tam_str > 0){
      Serial.print(" tamanho string recebida");
      Serial.println(tam_str);
      Serial.print("Conteudo:");
      Serial.println(str);
      handleroot();
    }
      
  } 
}




void handleroot(){
  Serial.print(str[0]);
  Serial.print(str[tam_str -2]);
  if ((str[0] == '>') && (str[tam_str-2] == '<')){
    for(size_t sel=0, j=0, i=1; i<tam_str ; i++){
      Serial.print("valor de j:");
      Serial.println(j);
      switch(sel){
        case(0):
          if (str[i] == ','){
            sel++;
            data.PEEP[j] = '\0';
            j=0;
          }
          else{
            data.PEEP[j] = str[i];
            j++;
          }
          break;
        case(1):
          if (str[i] == ','){
            sel++;
            data.PEEPmin[j] = '\0';
            j=0;
          }
          else{
            data.PEEPmin[j] = str[i];
            j++;
          }
          break;
        case(2):
          if (str[i] == ','){
            sel++;
            data.PEEPmax[j] = '\0';
            j=0;
          }
          else{
            data.PEEPmax[j] = str[i];
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
    Serial.println("Copia dos dados recebidos:");

    Serial.println(data.PEEP);
    Serial.println(data.PEEPmin);
    Serial.println(data.PEEPmax);
    Serial.println(data.volcorr);
    Serial.println(data.volcorrmax);
    Serial.println(data.volcorrmin);
    Serial.println(data.fluxo);
    Serial.println(data.fluxo_max);
    Serial.println(data.fluxo_min);
    Serial.println(data.id);
  }

  

}


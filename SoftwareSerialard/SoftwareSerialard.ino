/*
  Software serial multple serial test

 Receives from the hardware serial, dadoss to software serial.
 Receives from software serial, dadoss to hardware serial.

 The circuit:
 * RX is digital pin 10 (connect to TX of other device)
 * TX is digital pin 11 (connect to RX of other device)

 Note:
 Not all pins on the Mega and Mega 2560 support change interrupts,
 so only the following can be used for RX:
 10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69

 Not all pins on the Leonardo and Micro support change interrupts,
 so only the following can be used for RX:
 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).

 created back in the mists of time
 modified 25 May 2012
 by Tom Igoe
 based on Mikal Hart's example

 This example code is in the public domain.

 */
#include <SoftwareSerial.h>
#include <Arduino.h>

#define UPDATE_SERIAL_TIME 2000

bool serialFlag=false;

SoftwareSerial mySerial(10, 11); // RX, TX

static unsigned int timeCount=0;


//union dados_enviados{
//  struct {
//  short int PEEP=123, PEEPmin=456, PEEPmax=789;
//  short int volcorr=1011, volcorrmax=1213, volcorrmin=1415;
//  short int fluxo=1617, fluxo_max=1819, fluxo_min=2021;
//  short int id=1010;
//  };
//  short int envio[9];
//}dados;

struct {
  char PEEP[4]="0123", PEEPmin[4]="0456", PEEPmax[4]="0789";
  char volcorr[4]="1011", volcorrmax[4]="1213", volcorrmin[4]="1415";
  char fluxo[4]="1617", fluxo_max[4]="1819", fluxo_min[4]="2021";
  char id[4]="1010";
}dados;


void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Goodnight moon!");

  // set the data rate for the SoftwareSerial port
  mySerial.begin(4800);
  mySerial.println("Hello, world?");

    // Setup Timer 0
  TCCR0A = bit(WGM01);                // CTC operation
  TCCR0B = bit(CS01) | bit(CS00);     // pre-scaling 64
  OCR0A =  250;                       //  time = 250 * 64 * 1/16000000) = 1 ms
  TIMSK0 = bit (OCIE0A);              // interrupt on Compare A Match


}

ISR(TIMER0_COMPA_vect){
  if (timeCount==UPDATE_SERIAL_TIME){
    serialFlag=true;
    timeCount=0;
  }
  timeCount++;
}

void envio(){
  Serial.write(dados.PEEP);
  Serial.write(dados.PEEPmin); 
  Serial.write(dados.PEEPmax); 
  Serial.write(dados.volcorr); 
  Serial.write(dados.volcorrmax); 
  Serial.write(dados.volcorrmin); 
  Serial.write(dados.fluxo); 
  Serial.write(dados.fluxo_max); 
  Serial.write(dados.fluxo_min); 
  Serial.write(dados.id); 
}

void loop() { // run over and over
  if(serialFlag){
      delayMicroseconds(1000);
      envio();
    serialFlag=false;
  }
}

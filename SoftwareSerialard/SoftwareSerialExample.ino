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

SoftwareSerial mySerial(10, 11); // RX, TX

int read;

union dados_enviados{
  struct {
  short int PEEP, PEEPmin, PEEPmax;
  short int volcorr, volcorrmax, volcorrmin;
  short int fluxo, fluxo_max, fluxo_min;
  short int id;
  };
  short int envio[10];
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
}

void loop() { // run over and over
  
  dados.PEEP = 123;
  dados.PEEPmin = 789;
  dados.PEEPmax = 456;
  dados.volcorr = 1112;
  dados.volcorrmax = 1516;
  dados.volcorrmin = 1314;
  dados.fluxo = 1718;
  dados.fluxo_max = 1920;                               
  dados.fluxo_min = 2122;
  dados.id = 1010;
    delayMicroseconds(500);
    Serial.println("Valor enviado");
    for(int i=0; i<10 ; i++){
      delayMicroseconds(1000);
      mySerial.write(dados.envio[i]);
      Serial.println(dados.envio[i]);
    }
  delay(1);
}

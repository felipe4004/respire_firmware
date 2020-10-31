
#include <Arduino.h>
#include <U8g2lib.h>
#include <U8x8lib.h>
#include <SPI.h>
#include <EEPROM.h>
#include "oxy.h"
#include "oxy_eeprom.h"

 
/* ----------------------------------------------------------------------------------------
 *  SETUP
 */
void setup() {
  byte aux;
  
  // Definição das portas
  pinMode(TAC, INPUT_PULLUP);
  pinMode(PZER, INPUT_PULLUP);
  pinMode(KCONF, INPUT_PULLUP);
  pinMode(KARR, INPUT_PULLUP);
  pinMode(KINC, INPUT_PULLUP);
  pinMode(KDEC, INPUT_PULLUP);
  pinMode(LG, OUTPUT);
  pinMode(LR, OUTPUT);
  pinMode(MCLK, OUTPUT);
  pinMode(MDIR, OUTPUT);
  pinMode(BUZ, OUTPUT);
  pinMode(WTX, OUTPUT);
  pinMode(WRX, INPUT);
  pinMode(RS_CS, OUTPUT);
  
  // Estados inciais das portas
  digitalWrite(LG, LOW); 
  digitalWrite(LR, LOW);
  digitalWrite(MCLK, LOW); 
  digitalWrite(MDIR, LOW);  
  digitalWrite(WTX, LOW); 
  digitalWrite(RS_CS, HIGH); 
  //digitalWrite(BUZ, LOW);   
  
  // UART Comunicação serial com o computador
  Serial.begin(COM_PC_BR);

  // Ext0 interruption (Tacômetro)
  attachInterrupt (digitalPinToInterrupt (2), tacoISR, RISING);
  
  // Ext1 interruption (Inicio de curso)
  attachInterrupt (digitalPinToInterrupt (3), pZeroISR, RISING);

  // Pin change interrupt (Keyboard)
  PCMSK2 |= bit (PCINT20) | bit (PCINT21) | bit (PCINT22) | bit (PCINT23);  // PD4..PD7
  PCIFR  |= bit (PCIF2);              // clear any outstanding interrupts
  PCICR  |= bit (PCIE2);              // enable pin change interrupts for PCI2 

  // Setup Timer 0
  TCCR0A = bit(WGM01);                // CTC operation
  TCCR0B = bit(CS01) | bit(CS00);     // pre-scaling 64
  OCR0A =  250;                       //  time = 250 * 64 * 1/16000000) = 1 ms
  TIMSK0 = bit (OCIE0A);              // interrupt on Compare A Match

  // Setup Timer 1 & Motor
  resp.insSpe=300;
  resp.insPos=1010;
  resp.expSpe=200;
  resp.expPos=10;  
  TCCR1A = 0;                         // CTC operation, toggle OC1A on compare match if motor on
  TCCR1B = bit(WGM12) | bit(CS11) ;   // pre-scaling 8
  OCR1A = 40900;                      // max   
  TIMSK1 = bit (OCIE1A);              // interrupt on Compare A Match

  // Setup Timer 2 & Sound
  TCCR2A = bit(WGM21) ;               // CTC operation
  TCCR2B = bit(CS21) | bit(CS22);     // pre-scaling 256
  //TIMSK2 = 0 ;                        // interrupt disable. To sound on OCIE2A=1.
  sound(0);

  // ADC
  DIDR0 = 0x20;                                   // ADC5D=1 Digital Input of AD5 Disable, AD6 e AD7 don't have digital input
  ADMUX = 0x45;                                   // ADLAR=0 (left-adjust result), REF=1 AVCC with external capacitor at AREF pin, MUX3..0=5
  ADCSRB = bit(ADTS1) | bit(ADTS0);               //Auto Trigger by Timer0 
  ADCSRA |= bit (ADEN) | bit (ADIE)| bit(ADATE);  // ADC enable, Interrupt enable, Auto Trigger Enable

  // LCD 
  u8g2.begin();
  u8g2.setFont(u8g2_font_t0_11_tr);
  for(byte i=0;i<88;i++){
    plog[i]=28;
    flog[i]=63;    
  }
}


/* ----------------------------------------------------------------------------------------
 *  Tacômetro - Int0 ISR
 */ 
// ISR Int0 (Tachometer sensor)
void tacoISR () {
  led(LGREEN); // apenas para teste
   
}

/* ----------------------------------------------------------------------------------------
 *  Sesor de inicio de curso - Int1 ISR
 */ 
void pZeroISR () {
  led(LRED);  // apenas para teste
  mot.pos=0;
}

/* ----------------------------------------------------------------------------------------
 *  Teclado - Pin changue ISR
 *  Keys KConf, KArr, KInc, KDec
 */
ISR (PCINT2_vect) {
  static byte portReg1=0xF0; // last port value
  byte portReg2;             // current port value
  portReg2 = PIND & 0xF0;
  if (portReg2<portReg1) {  // If any key was pressed
    debounceCount=KEYB_DEBOUNCE;// reset debounce count
    portReg1=portReg2;
  }
  else {                    // If any key was released  
    if (debounceCount==0) {
      KeyLedState=(KeyLedState & 0xF0) | (((~portReg1)>>4) & 0x0F) ;
    }
    portReg1=0xF0;
  }
  led(LRED);  // apenas para teste
}

/* ----------------------------------------------------------------------------------------
 *  Timer0 ISR
 */
ISR(TIMER0_COMPA_vect){
  static unsigned int timeCount=0;
  static byte sampleTimeCount=SAMPLE_TIME;
  static byte ledTimeCount=LED_TIME;
  static byte motSta;
  if (timeCount & 0x01){  // Incrementa debounceCount cada 2 intervalos de timeCount
    if (debounceCount>0){ // se debounceCount for > 0
      debounceCount--;
    }
  }
  if(sampleTimeCount==0){
    sampleTimeCount=SAMPLE_TIME;
    sendDat=1;
  }
  if(motSta!=mot.sta){
    motSta=mot.sta;
    ledTimeCount=LED_TIME;
  }
  if(ledTimeCount>0){
    led(LGREEN);
    ledTimeCount--;
  }  
  else{
    led(LOFF);
  }
  if (timeCount==UPDATE_LCD_TIME){
    lcdFlag=1;
    timeCount=0;
  }
  timeCount++;
  sampleTimeCount--;
}


/* ----------------------------------------------------------------------------------------
 *  Timer1 ISR
 */
ISR(TIMER1_COMPA_vect){
  static byte divCount=0;
  static byte rampCount=0;
  divCount++;
  if (divCount==2*PASS_DIV) {
    divCount=0;
    if (mot.pos==mot.posSP){
      bitClear(mot.sta,0);  // motor off
      bitClear(TCCR1A,COM1A0); // saída OC1A desligada
      rampCount=0;
    }
    else{
      bitSet(mot.sta,0);  // motor on
      bitSet(TCCR1A,COM1A0); // saída OC1A ligada
      if (mot.pos<mot.posSP){
        bitClear(mot.sta,1);  // mov +
        digitalWrite(MDIR, LOW);
        mot.pos++;
      }
      else {
        bitSet(mot.sta,1);  // mov -
        digitalWrite(MDIR, HIGH);
        mot.pos--;
      }
//      // Rampa de aceleração/desaceleração
//      rampCount++;
//      if (rampCount==mot.ramp){
//          rampCount=0;
//        if (mot.spe!=mot.speSP){
//          if (mot.spe<mot.speSP){
//            mot.spe++;
//          }
//          else{
//            mot.spe--;
//          }
//          mot.ocr=speed2OCR(mot.spe);
//        }
//      } 
//      
//      setOCR(mot.spe);
    }
  }
}

/* ----------------------------------------------------------------------------------------
 *  Timer2 ISR
 */
ISR(TIMER2_COMPA_vect){
  if(bitRead(KeyLedState,4)){
    bitClear(KeyLedState,4);
    digitalWrite(BUZ, LOW);
  }
  else{
    bitSet(KeyLedState,4);
    digitalWrite(BUZ, HIGH);
  }
}

/* ----------------------------------------------------------------------------------------
 *  ADC complete conversion ISR
 *  Sequencial reading of A5 to A8 
 *  Trigger by Timer0
 */
ISR (ADC_vect) {
  byte low, high;
  static byte state=0;
  // we have to read ADCL first; doing so locks both ADCL
  // and ADCH until ADCH is read.  reading ADCL second would
  // cause the results of each conversion to be discarded,
  // as ADCL and ADCH would be locked when it completed.
  low = ADCL;
  high = ADCH;
  daq.adcValue[state] = (high << 8) | low;
  state++;
  if (state==3) {
    state=0;
  }
  ADMUX &= 0x70;
  ADMUX |= (state+5);
}

/* ----------------------------------------------------------------------------------------
 *  setOCR
 *  Conversion stepper motor speed to OCR
 */
void setOCR(unsigned int spe) {
  // OCR=(Coef/(2*DivuP*Speed))-OCRmin
  OCR1A=(CONV_SPEED_OCR/((PASS_DIV<<1)*spe))-OCR_MIN;
}

/* ----------------------------------------------------------------------------------------
 *  Sound
 *  Activa ou desativa interrução do Timer2
 *  val : frequência do som
 *  freq = 16000000 / ( 512 * ( 15 + val ))  [Hz] 
 */
void sound(byte val){
  if(val==0){
    TIMSK2 = 0; // Soun off
    digitalWrite(BUZ, LOW);
  } 
  else{
    OCR2A = 14 + (val & 0x7F);
    TIMSK2 = bit(OCIE2A); // Sound on
  }
}

/* ----------------------------------------------------------------------------------------
 *  Led
 *  Liga ou desliga o led 
 *  val = LOFF desligados
 *  val = LGREEN verde ligado
 *  val = LRED vermelho ligado
 */ 
void led(byte val) {
  switch (val){
    case LOFF:
      digitalWrite(LG, LOW); 
      digitalWrite(LR, LOW);  
      bitClear(KeyLedState,LGREEN);  
      bitClear(KeyLedState,LRED);
      break;
    case LGREEN:
      digitalWrite(LG, HIGH);   
      digitalWrite(LR, LOW);  
      bitSet(KeyLedState,LGREEN);
      bitClear(KeyLedState,LRED);
      break;
    case LRED:
      digitalWrite(LG, LOW); 
      digitalWrite(LR, HIGH); 
      bitClear(KeyLedState,LGREEN);
      bitSet(KeyLedState,LRED);      
      break;          
  }
}

/* ----------------------------------------------------------------------------------------
 *  measureProc
 *  Procesamento das medições
 */
void measureProc(){
  daq.pres = daq.adcValue[2];
  daq.flow = daq.adcValue[0];
  daq.vBat = daq.adcValue[1];
  
  // Apenas para teste, falta o processamento real
  daq.temp = random(10, 40);
  daq.volCorr = random(350,700);
  daq.volMinu = random(350,700);
  daq.freq = random(10,30);
  daq.presPla= random(35,210);
}

/* ----------------------------------------------------------------------------------------
 *  respControl
 *  Controle da respiração
 */
void respControl(){
  static byte stateControl=0;
  if(mot.pos==mot.posSP){ // Se o setpoint de posição foi atingido
    stateControl++;       // muda estado
    if(stateControl==2){
      stateControl=0;
    }
    switch (stateControl){
      case 0:   // Inspiração
        mot.posSP = resp.insPos;
        setOCR(resp.insSpe);
        mot.speSP=resp.insSpe;
        break;
      case 1:   // Expiração
        mot.posSP = resp.expPos;
        setOCR(resp.expSpe);
        mot.speSP=resp.expSpe;
        break;
    }
    mot.spe=mot.speSP;  
  }
}

void transf(char *str, long v){
  byte i;
  sprintf(str,"%02d",v);
  for(i=0;i<=sizeof(str);i++){
    if(str[i]==0){
      break;
    }
  }
  str[i]=str[i-1];
  str[i-1]='.';
  str[i+1]=0;
}
      
/* ----------------------------------------------------------------------------------------
 *  Main loop
 */
void loop() {
  char c;
  char str[10];
  long v;
  byte i;
  
  if (sendDat){
    measureProc();
    respControl();
//    Serial.print (">");
//    Serial.print (daq.pres);
//    Serial.print (",");
//    Serial.print (daq.flow);
//    Serial.print (",");
//    Serial.print (daq.vBat);
//    Serial.print (",");
//    Serial.print (daq.temp);
//    Serial.print (",");
//    Serial.print (daq.volCorr);
//    Serial.print (",");
//    Serial.print (daq.volMinu);
//    Serial.print (",");
//    Serial.print (daq.freq);
//    Serial.print (",");
//    Serial.print (daq.presPla);
//    Serial.print (",");
//    Serial.print (mot.pos);
//    Serial.print (",");
//    Serial.print (mot.posSP);
//    Serial.print (",");
//    Serial.print (mot.spe);
//    Serial.print (",");
//    Serial.print (mot.speSP);
//    Serial.print (",");
//    Serial.print (KeyLedState,BIN);
//    Serial.println ();
    sendDat=0;
  }
//  while (Serial.available() > 0) {
//    c = Serial.read();      // read from Serial Monitor
//    u8g2log.print(c);               // print to display
//    Serial.print(c);                // and print back to monitor
//  }
  if (lcdFlag==1){
    u8g2.firstPage();
    do {
      u8g2.drawLine(38, 0, 38, 63);
      //u8g2.drawLine(38, 31, 127, 31);
      // Pressure
      v=((long)daq.pres*400)/1023;
      transf(str,v);
      i=u8g2.getStrWidth(str);
      u8g2.drawStr(38/2-i/2,15,str);
      u8g2.drawStr(0,25,"cm H2O");
      for(i=0;i<88;i++){
        plog[i]=plog[i+1];
      } 
      v=(v*26)/400;
      plog[88]=27-(byte)v;
      for(i=0;i<88;i++){
        u8g2.drawLine(i+39,plog[i],i+40,plog[i+1]);    
      }
      // Flow      
      v=((long)daq.flow*700)/1023;
      transf(str,v);
      i=u8g2.getStrWidth(str);
      u8g2.drawStr(38/2-i/2,48,str);
      u8g2.drawStr(0,60,"mL/min");
      for(i=0;i<88;i++){
        flog[i]=flog[i+1];
      } 
      v=(v*26)/400;
      flog[88]=63-(byte)v;
      for(i=0;i<88;i++){
        u8g2.drawLine(i+39,flog[i],i+40,flog[i+1]);    
      }
      v=abs((mot.pos-resp.expPos)*38/(resp.insPos-resp.expPos));
      u8g2.drawBox(0,30,(int)v,3);
      switch(KeyLedState&0x0F){
        case 1:
          u8g2.drawStr(45,50,"Bt 2 Pressed");
          break;
        case 2:
          u8g2.drawStr(45,50,"Bt 1 Pressed");
          break;
        case 4:
          u8g2.drawStr(45,50,"Bt 4 Pressed");
          break;
        case 8:
          u8g2.drawStr(45,50,"Bt 3 Pressed");
          break;
        case 3:        
          u8g2.drawStr(45,50,"Bt 1&2 Pressed");
          break;
        case 10:        
          u8g2.drawStr(45,50,"Bt 1&3 Pressed");
          break;
        case 6:        
          u8g2.drawStr(45,50,"Bt 1&4 Pressed");
          break;    
        case 9:        
          u8g2.drawStr(45,50,"Bt 2&3 Pressed");
          break;    
        case 5:        
          u8g2.drawStr(45,50,"Bt 2&4 Pressed");
          break;
        case 12:        
          u8g2.drawStr(45,50,"Bt 3&4 Pressed");
          break;
      }
    } while ( u8g2.nextPage() );
    lcdFlag=0;
  }
  // 
  //sound(byte(daq.adcValue[2]>>3));
  //
  
}

#include "respire.h"


U8GLIB_ST7920_128X64_1X u8g(13, 11, A1);

SoftwareSerial serialsoft(4, A4); //rx, tx soft



void setup(void) {
  // Setup Timer 0
  TCCR0A = bit(WGM01);                // CTC operation
  TCCR0B = bit(CS01) | bit(CS00);     // pre-scaling 64
  OCR0A =  250;                       //  time = 250 * 64 * 1/16000000) = 1 ms
  TIMSK0 = bit (OCIE0A);              // interrupt on Compare A Match

  // Setup Timer 1 & Motor
  TCCR1A = 0;                         // CTC operation, toggle OC1A on compare match if motor on
  TCCR1B = bit(WGM12) | bit(CS11) | bit(CS10);   // pre-scaling 64
  OCR1A = 5;                      // time = 5*64*(1/16000000) = 20 us   
  TIMSK1 = bit (OCIE1A);              // interrupt on Compare A Match

    // Interrupcao externa fim de curso
  attachInterrupt (digitalPinToInterrupt (FCOURSEMIN), fcourse, RISING);
  attachInterrupt (digitalPinToInterrupt (FCOURSEMAX), fcourse, RISING);

  pinMode (DT, INPUT);
  pinMode (CLK, INPUT);
  pinMode (SW, INPUT);


  pinMode (PUL, OUTPUT);
  pinMode (DIR, OUTPUT);
  //pinMode (EN, OUTPUT);
  //digitalWrite(EN,HIGH);


  Serial.begin(115200);
  serialsoft.begin(115200);

    // put your setup code here, to run once:
}


void loop(void) {

  if(configFlag == true){
    
    u8g.firstPage();

    //tela de configuracoes

    do{
      drawConfig();
    }while(u8g.nextPage());
  }

  else{ 
    int i=0;

  u8g.firstPage(); //Manutencao da figura no display
  measureFunction(); //Funcao para a medicao ADC
  serialSend();


    if(lcdFlag){
      
      //Escrita no display


      for(i=0; i < 63; i++){
        p_plot[i] = p_plot[i+1];
        f_plot[i] = f_plot[i+1];
      }
      p_plot[i] = (int)(64.0-(53.0*daq.pres/1023.0));
      f_plot[i] = (int) (64.0-(53.0*daq.flow/1023.0));

      do {
        draw();
      } while( u8g.nextPage() );
      lcdFlag = false;
    }
  }
  

}


/* 
*  Timer0 ISR
*/
ISR(TIMER0_COMPA_vect){
  static unsigned int timeCount=0;
  static int lastCLK;
  static int n;
/*   static unsigned int lastRot=0;
 */  if (timeCount==UPDATE_LCD_TIME){
    lcdFlag=true;
    timeCount=0;
  }
  timeCount++;


  n = digitalRead(CLK);
  if ((lastCLK == LOW) && (n == HIGH)) {
    if (digitalRead(DT) == LOW) {
      selection--;
    } else {
      selection++;
    }
    Serial.print (selection);
    Serial.println ("/");
  }
  lastCLK = n;


}

/* 
 *  Timer1 ISR
 */
ISR(TIMER1_COMPA_vect){
  static unsigned int interval=0;
  static unsigned int ramp = 300;
  static unsigned int countRamp=0;



  switch(stateMotor){
    case(0):      //define a direcao
      daq.step = 0;
      interval = 0;
      stateMotor = 1;
      countRamp=0;
      ramp=300;

      digitalWrite(DIR, LOW);
    case(1):      //coloca o pulso em nivel alto e aguarda 20us
      digitalWrite(PUL, HIGH);
      stateMotor = 2;
      break;
    case(2):      //coloca em nivel baixo e verifica se ja chegaram nas 20000 contagens
      if(countRamp < ramp){
        countRamp++;
        break;
      }
      else{
        digitalWrite(PUL,LOW);
        daq.step++;
        ramp--;
        daq.step < 30000 ? stateMotor=1 : stateMotor = 3;
        break;
      }

    case(3):      //intervalo de 10 ms
      interval++;
      interval < 500 ? stateMotor = 3 : stateMotor = 4;
      break;
    case(4):      //define a direcao
      digitalWrite(DIR,HIGH);
      daq.step = 0;
      interval = 0;
      stateMotor = 5;
      countRamp = 0;
      ramp = 300;
    case(5):      //coloca o pulso em nivel alto e aguarda 20us
      digitalWrite(PUL, HIGH);
      stateMotor = 6;
      break;
    case(6):      //coloca o pulso em nivel baixo e verifica a condicao
        if(countRamp < ramp){
        countRamp++;
        break;
      }
      else{
        digitalWrite(PUL,LOW);
        daq.step++;
        ramp--;
        daq.step < 30000 ? stateMotor=5 : stateMotor = 7; //vai para o 7 se ja for atingido o valor de 20000
        break;
      }
      break;
    case(7):
      interval++;
      interval < 500 ? stateMotor = 7 : stateMotor = 0;
    default:
      break;

      
  }

}

void u8g_prepare(void) {
  //size_t i;
  u8g.setColorIndex(1);
  u8g.setFont(u8g_font_6x10);
  u8g.setFontRefHeightExtendedText();
  u8g.setDefaultForegroundColor();
  u8g.setFontPosTop();


}

void u8g_frame(void) {

  u8g.drawLine(0, 11, 0, 11+52);
  u8g.drawLine(0, 11+52, 126/2, 11+52);
  u8g.drawLine(128/2+2,11, 128/2+2, 11+52);
  u8g.drawLine(128/2+2, 11+52, 128, 11+52);
}


void u8g_string(void) {
  uint8_t t;
  sprintf(str, "cmH2O    L/min");
  t=u8g.getStrWidth(str);
  u8g.drawStr(63 - t/2,0, str);
}



void draw(void) {
  int j=0;
  u8g_prepare();
  u8g_string();
  for(j=0; j<63; j++){
    u8g.drawLine(j, p_plot[j], j+1, p_plot[j+1]);
    u8g.drawLine(65+j, f_plot[j], 65+j+1, f_plot[j+1]);
  }
  u8g_frame();
}


void measureFunction(void){
    daq.flow = analogRead(A5);
    daq.pres = analogRead(A7);

}

void fcourse(){

  if(digitalRead(FCOURSEMAX)){
    stateMotor = 4;
  }
  else{
    stateMotor = 0;
  }
}

void serialSend(){

  sprintf(str, ">%hi,%hi,%d<;", daq.pres, daq.flow, daq.step);
  serialsoft.write(str);

}

void drawConfig(void){
  char selArray[10];
  sprintf(selArray,"%d", selection);
  const char *aux = selArray;

  u8g_prepare();

  drawFrameConfig();
  drawStringsConfig();

  switch (selConfig)
  {
  case 0:
    if(SW == 0){
      selConfig++;
      selection = 0;
      break;
    }
    else{
      confVol(aux);
      break;
    }

  case 1:
    if(SW == 0){
      selConfig++;
      selection = 0;
      break;
    }
    else{
      confPres(aux);
      break;
    }
  case 2:
    if(SW == 0){
      selConfig++;
      selection = 0;
      configFlag = false;
      break;
    }
    else{
      confFr(aux);
      break;
    }

  default:
    selConfig = 0;
    break;
  }

}

void drawFrameConfig(){
  u8g.drawLine(0, 10, 127, 10);
}

void drawStringsConfig(){
  uint8_t t;
  
  t = u8g.getStrWidth("CONFIGURAÇÕES");
  u8g.drawStr((63 - t/2), 10, "CONFIGURAÇÕES");
  
  t = u8g.getStrWidth("Vol:");
  u8g.drawStr((31 - t/2), 25, "Vol.:");
  
  t = u8g.getStrWidth("Pres.:");
  u8g.drawStr((63-t/2), 25, "Pres.:");

  t = u8g.getStrWidth("F. Resp.:");
  u8g.drawStr((95 - t/2), 25, "F.Resp.:");
}


void confVol(const char *selection){
  uint8_t t = 0;

  t = u8g.getStrWidth(selection);
  u8g.drawStr(31 - t/2, 35, selection);
}

void confPres(const char *selection){
  uint8_t t = 0;

  t = u8g.getStrWidth(selection);
  u8g.drawStr(63 - t/2, 35, selection);
}

void confFr(const char *selection){
  uint8_t t = 0;

  t = u8g.getStrWidth(selection);
  u8g.drawStr(95 - t/2, 35, selection);
}





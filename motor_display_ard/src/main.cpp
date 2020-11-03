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


  pinMode (PUL, OUTPUT);
  pinMode (DIR, OUTPUT);
  //pinMode (EN, OUTPUT);
  //digitalWrite(EN,HIGH);


  Serial.begin(115200);
  serialsoft.begin(115200);

    // put your setup code here, to run once:
}


void loop(void) {
  
  int i=0;
  
  u8g.firstPage(); //Manutencao da figura no display
  measureFunction(); //Funcao para a medicao ADC


  if(lcdFlag){
    
    //Escrita no display


    for(i=0; i < 63; i++){
      p_plot[i] = p_plot[i+1];
      f_plot[i] = f_plot[i+1];
    }
    p_plot[i] = (int)(64.0-(53.0*daq.pres/500.0));
    f_plot[i] = (int) (64.0-(53.0*daq.flow/750.0));

  do {
    draw();
  } while( u8g.nextPage() );
  lcdFlag = false;
  }

}


/* 
*  Timer0 ISR
*/
ISR(TIMER0_COMPA_vect){
  static unsigned int timeCount=0;
  if (timeCount==UPDATE_LCD_TIME){
    lcdFlag=true;
    timeCount=0;
  }
  timeCount++;

}

/* 
 *  Timer1 ISR
 */
ISR(TIMER1_COMPA_vect){
  static unsigned int interval=0;
  static unsigned int ramp = 300;
  static unsigned int countRamp=0;

  serialSend();


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





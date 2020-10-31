#include "respire.h"


U8GLIB_ST7920_128X64_1X u8g(13, 11, A1);


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


  pinMode (PUL, OUTPUT);
  pinMode (DIR, OUTPUT);
  //pinMode (EN, OUTPUT);
  //digitalWrite(EN,HIGH);


  Serial.begin(9600);

    // put your setup code here, to run once:
}


void loop(void) {
  
  u8g.firstPage(); //Manutencao da figura no display

  if(lcdFlag){

    measureFunction(); //Funcao para a medicao ADC

    //Escrita no display
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
  static unsigned int steps=0;
  static unsigned int interval=0;
  static int stateMotor = 0;

  switch(stateMotor){
    case(0):      //define a direcao
      steps = 0;
      interval = 0;
      stateMotor = 1;
      digitalWrite(DIR, LOW);
    case(1):      //coloca o pulso em nivel alto e aguarda 20us
      digitalWrite(PUL, HIGH);
      stateMotor = 2;
      break;
    case(2):      //coloca em nivel baixo e verifica se ja chegaram nas 20000 contagens
      digitalWrite(PUL,LOW);
      steps++;
      steps < 20000 ? stateMotor=1 : stateMotor = 3;
      break;
    case(3):      //intervalo de 10 ms
      interval++;
      interval < 250 ? stateMotor = 3 : stateMotor = 4;
      break;
    case(4):      //define a direcao
      digitalWrite(DIR,HIGH);
      steps = 0;
      interval = 0;
      stateMotor = 5;
    case(5):      //coloca o pulso em nivel alto e aguarda 20us
      digitalWrite(PUL, HIGH);
      stateMotor = 6;
      break;
    case(6):      //coloca o pulso em nivel baixo e verifica a condicao
      digitalWrite(PUL,LOW);
      steps++;
      steps < 20000 ? stateMotor=5 : stateMotor = 0; //retorna para o se ja for atingido o valor de 20000
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
    int i = 0;
    daq.flow = (int) (64.0-(53.0*analogRead(A5)/750.0));
    daq.pres = (int) (64.0-(53.0*analogRead(A7)/500.0));

    // insercao de um novo valor no vetor

    for(i=0; i < 63; i++){
      p_plot[i] = p_plot[i+1];
      f_plot[i] = f_plot[i+1];
    }
    p_plot[i] = daq.pres;
    f_plot[i] = daq.flow;

}

#include <U8glib.h>
#include <Arduino.h>

// IMPORTANT NOTE: The following list is incomplete. The complete list of supported 
// devices with all constructor calls is here: https://github.com/olikraus/u8glib/wiki/device
//Configuração de Pinagem, Enable, RW, RS, RESET
U8GLIB_ST7920_128X64_1X u8g(13, 11, A1);


#define UPDATE_LCD_TIME 40

struct daq_type {
  volatile unsigned int adcValue[3];
  int           pres=10;
  int           flow=10;
  byte          vBat;
  int           temp;
  int           volCorr;
  unsigned int  volMinu;
  int           freq;
  int           presPla;
} daq;  

int p_plot[64];
int f_plot[64];
uint8_t draw_state = 0;
uint8_t page_sel = 0;
char str[127];
bool lcdFlag=false;
unsigned int timeCount=0;
size_t i = 0;
double v[1];



void u8g_prepare(void) {
  //size_t i;
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


void u8g_string(char *str) {
  uint8_t t;
  sprintf(str, "cmH2O    L/min");
  t=u8g.getStrWidth(str);
  u8g.drawStr(63 - t/2,0, str);
}



void draw(void) {
  int j=0;
  u8g_prepare();
  u8g_string(str);
  for(j=0; j<63; j++){
    u8g.drawLine(j, p_plot[j], j+1, p_plot[j+1]);
    u8g.drawLine(65+j, f_plot[j], 65+j+1, f_plot[j+1]);
  }
  u8g_frame();
}

/* ----------------------------------------------------------------------------------------
*  Timer0 ISR
*/
ISR(TIMER0_COMPA_vect){
  if (timeCount==UPDATE_LCD_TIME){
    lcdFlag=true;
    timeCount=0;
  }
  timeCount++;

}


void setup(void) {
  // Setup Timer 0
  TCCR0A = bit(WGM01);                // CTC operation
  TCCR0B = bit(CS01) | bit(CS00);     // pre-scaling 64
  OCR0A =  250;                       //  time = 250 * 64 * 1/16000000) = 1 ms
  TIMSK0 = bit (OCIE0A);              // interrupt on Compare A Match

  Serial.begin(9600);
  if ( u8g.getMode() == U8G_MODE_R3G3B2 )
    u8g.setColorIndex(20);
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT )
    u8g.setColorIndex(1);
  else if ( u8g.getMode() == U8G_MODE_BW )
    u8g.setColorIndex(1);
    // put your setup code here, to run once:
}


void loop(void) {
  
  u8g.firstPage();
  if(lcdFlag){

    daq.flow = (int) (64.0-(53.0*analogRead(A5)/1023.0));
    daq.pres = (int) (64.0-(53.0*analogRead(A7)/1023.0));

    for(i=0; i < 63; i++){
      p_plot[i] = p_plot[i+1];
      f_plot[i] = f_plot[i+1];
    }
    p_plot[i] = daq.pres;
    f_plot[i] = daq.flow;
/*     Serial.println("f_plot:");
    Serial.println(f_plot[i]);
    Serial.println("daq.flow");
    Serial.println(daq.flow); */
  
  do {
    draw();
  } while( u8g.nextPage() );
  lcdFlag = false;
  }
}
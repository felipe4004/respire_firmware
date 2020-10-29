#include <U8glib.h>
#include <Arduino.h>

// IMPORTANT NOTE: The following list is incomplete. The complete list of supported 
// devices with all constructor calls is here: https://github.com/olikraus/u8glib/wiki/device
//U8GLIB_ST7920_128X64_4X u8g(8, 9, 10, 11, 4, 5, 6, 7, 18, 17, 16);   // 8Bit Com: D0..D7: 8,9,10,11,4,5,6,7 en=18, di=17,rw=16
U8GLIB_ST7920_128X64_1X u8g(18, 16, 17);	// SPI Com: SCK = en = 18, MOSI = rw = 16, CS = di = 17

#define UPDATE_LCD_TIME 200

struct daq_type {
  volatile unsigned int adcValue[3];
  int           pres;
  int           flow;
  byte          vBat;
  int           temp;
  unsigned int  volCorr;
  unsigned int  volMinu;
  int           freq;
  int           presPla;
} daq;  

uint8_t p_plot[123];
uint8_t f_plot[123];
uint8_t volcorr_plot [123];
uint8_t draw_state = 0;
uint8_t page_sel = 0;
char str[127];
unsigned short int lcdFlag=0;
static unsigned int timeCount=0;



void u8g_prepare(void) {
  uint8_t i;
  u8g.setFont(u8g_font_6x10);
  u8g.setFontRefHeightExtendedText();
  u8g.setDefaultForegroundColor();
  u8g.setFontPosBaseline();

  for(i=0; i < 122; i++){
    p_plot[i] = p_plot[i+1];
  
  p_plot[i+1] = daq.pres;
  for(i=0; i<122; i++)
    f_plot[i] = f_plot[i+1];
  
  f_plot[i+1] = daq.flow;
  for(i=0; i<122; i++)
    volcorr_plot[i] = volcorr_plot[i+1];

  volcorr_plot[i+1] = daq.volCorr;
  
  }
}

void u8g_frame(uint8_t a) {
  u8g.drawFrame(5,12,122,59);
}


void u8g_string(char *str, const char *unit ) {
  uint8_t t;
  sprintf(str, "val:...%s", unit);
  t=u8g.getStrWidth(str);
  u8g.drawStr(63 - t/2,12, str);
}


void u8g_line(uint8_t *plot) {
  uint8_t i;
  for(i=0; i<122; i++)
    u8g.drawLine(i+6, plot[i], i+7, plot[i+1]);
} 


void draw(void) {
  u8g_prepare();
  switch(draw_state) {
    case 0: 
      u8g_string(str, "cmH2O");
      u8g_line(p_plot);
      break;
    case 1: 
      u8g_string(str, "L/min"); 
      u8g_line(f_plot);
      break;
    case 2: 
      u8g_string(str, "L"); 
      u8g_line(volcorr_plot);
      break;

  }
}

/* ----------------------------------------------------------------------------------------
 *  Timer0 ISR
 */
ISR(TIMER0_COMPA_vect){
  if (timeCount==UPDATE_LCD_TIME){
    lcdFlag=1;
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

#if defined(ARDUINO)
  pinMode(13, OUTPUT);           
  digitalWrite(13, HIGH);  
#endif

  for(uint8_t i=0; i <= 122; i++){
    f_plot[i] = 20;
    p_plot[i] = 20;

  }
}


void loop(void) {
  
  if(lcdFlag==1){

    daq.flow = random(12, 60);
    daq.pres = random(12, 60);
    daq.volCorr = random(12, 60);

    u8g.firstPage();
  
  do {
    draw();
  } while( u8g.nextPage() );
  
  // increase the state
  if(page_sel == 50)
    draw_state++;
  if ( draw_state >= 3 )
    draw_state = 0;
  
  }
}
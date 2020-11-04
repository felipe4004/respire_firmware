#ifndef _RESPIRE_H_
#define _RESPIRE_H_

#include <Arduino.h>
#include <U8glib.h>
#include <SoftwareSerial.h>


/*-------------------MOTOR------------------*/

#define PUL 9
#define DIR 10
//#define EN    //Nao sei o que significa esse pino 

volatile uint8_t stateMotor = 0;


#define FCOURSEMIN  2
#define FCOURSEMAX  3


/*---------------------ROTARY ENCODER--------*/

#define DT 6
#define CLK 7
#define SW 5

bool sw=false;
signed int selection=0;



/*--------------------LCD-----------------*/
//Configuração de Pinagem, Enable, RW, RS, RESET



#define UPDATE_LCD_TIME 40


//Declaracao das variaveis

int p_plot[64];
int f_plot[64];
bool lcdFlag;
char str[127];



/*-----------------------MEDICOES*-------------*/
/*
 *  MEASUREMENTS
 *  daq.adcValue  : Array com valores em unidades de conversão do fluxo do ar (0), tensão elétrica na bateria (1), 
 *                  e pressão do ar (2) (0-2013) e temperatura interna do microcontrolador (3). 
 *  daq.pres      : Valor de pressão do ar (+/-50.0 cmH2O). Ponto fixo, uma casa decimal.
 *  daq.flow      : Valor de fluxo do ar (+/- 21.0 L/min ). Ponto fixo, uma casa decimal.
 *  daq.vBat      : Tensão elétrica na bateria (0.0-14.0 V). Ponto fixo, uma casa decimal.
 *  daq.temp      : Temperatura do ar (0.0-50.0 °C). Ponto fixo, uma casa decimal.
 *  daq.volCorr   : Volume corrente de ar (0-700 mL). Inteiro
 *  daq.volMinu   : Volume / minuto de ar (0-21000 mL/min). Inteiro
 *  daq.freq      : Frequência respiratória medida em respiração asistida [0.0-30.0 Ciclos/min]. Ponto fixo, uma casa decimal.
 *  daq.presPla   : Pressão de platô (+/-50.0 cmH2O). Ponto fixo, uma casa decimal.
 *  
 *  As medições obtidas do ADC (apenas os valores 0-2 do adcValue) devem ser convertidas a unidades de engenharia 
 *  no formato definido e armazenadas na variável correspondente. 
 *  As outras medições serão calculadas no módulo de processamento ou obtidas por outros métodos.
 */

struct daq_type {
    int           pres;
    int           flow;
    byte          vBat;
    volatile unsigned int  step;
    int           volCorr;
    unsigned int  volMinu;
    int           freq;
    int           presPla;
} daq;





/*------------PROTOTIPOS DE FUNCOES----------------*/

void u8g_prepare(void);
void u8g_frame(void);
void u8g_string(void);
void draw(void);
void measureFunction(void);
void fcourse(void);
void serialSend();


#endif
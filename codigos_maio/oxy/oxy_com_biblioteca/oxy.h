#ifndef _HEADERFILE_H
#define _HEADERFILE_H

#include "oxy_types.h"
/* ----------------------------------------------------------------------------------------
 *  I/O PORTS
*/
#define TAC     2
#define PZER    3
#define KCONF   4
#define KARR    5
#define KINC    6
#define KDEC    7
#define LG      8
#define LR      A0
#define BUZ     12
#define RW_DAT  11
#define E_CLK   13
#define RS_CS   A1
#define TEMP    A2
#define MCLK    9
#define MDIR    10
#define WRX     A3
#define WTX     A4

/* ----------------------------------------------------------------------------------------
 *  KEYBOARD AND LEDs
 * 
 * KEYB_DEBOUNCE : Tempo de debounce (0-255). Seu valor vezes 2 vai especificar o tempo em ms.
 *                 0 - sem debounce, 255 - 510 ms
 * 
 * debounceCount : Contador do tempo de debounce.
 * 
 * KeyLedState: Os 4 bits menos significativos indicam qual botão ou botões foram pressionados.
 *            Quando liberado, se for completado o tempo de debounce pré-definido, 
 *            keybState terá em '1' os bits do botão ou botões pressionados, conforme lista a seguir:          
 * Bit 0      CONF      
 * Bit 1      ARR
 * Bit 2      INC
 * Bit 3      DEC
 *            Se foram pressionados vários botões dentro de um período de debounce, 
 *            o tempo de debounce será contabilizado no intervalo estabelecido pelo último botão 
 *            pressionado e o primeiro liberado.
 *            Para determinar se um botão foi alterado, pode-se usar if ((KeyLedState&ox0F)!=0) 
 *            e depois, se é verdadeiro, verifcar qual botão foi pressionado.
 *            Depois de isso os 4 bits devem ser apagados para evitar uma nova deteção.
 *            Exemplos: xxxx 1000 CONF foi pressionado, xxxx 1010 CONF e INC foram pressionados
 *            
 *            Os 2 bits mais signicativos indicam o estado dos LEDs.
 *            Quando o bit de um LED está em '1', o led está ligado.
 *Bit 6       Led Verde
 *Bit 7       Led Vermelho
 *
 *Bit 4       Oscilador buzzer (usado em Timer2)
 */
#define KEYB_DEBOUNCE 100  // 200 ms
//volatile byte debounceCount=KEYB_DEBOUNCE;
//volatile byte KeyLedState=0;
#define PCONF   0 // The key CONF was pressed
#define PARR    1 // The key ARR was pressed
#define PINC    2 // The key INC was pressed
#define PDEC    3 // The key DEC was pressed
#define LOFF    0 // The LEDs are off
#define LGREEN  6 // The LED green is on
#define LRED    7 // The LED red is on

/* ----------------------------------------------------------------------------------------
 *  BUZZER
 *  
 *  sound(valor) : Liga o som na frequência especificada nos 7 bits menos significativos (0-127) 
 *                 Valor zero desliga o som.
 *                 freq = 16000000 / ( 512 * ( 15 + valor ))  [Hz] 
 *                 Faixa: 221.6 -2083 Hz
 */


/* ----------------------------------------------------------------------------------------
 *  MOTOR
 *  
 */
#define MOTOR_TYPE 0       // 0 Stepper Motor, 1 CC Motor
#define SMOTOR_STEP_DIV1 1 // LSB Stepper Motor Driver Step Divisor
#define SMOTOR_STEP_DIV2 2 //     (1, 2, 4, 8, 16, 32, 64, 128) 
#define SMOTOR_STEP_DIV3 3 // MSB Stepper Motor Driver Step Divisor 


struct motor_type{
  volatile byte          sta;    // 
  volatile unsigned long pos;    // Contador de posição (passos ou pulsos do tacómetro) (0-65535)
  volatile unsigned long posSP;  // Set point de posição (0-65535)
  volatile int           spe;    // Velocidade (0-1023)
  int                    speSP;  // Set point de velocidade (0-1023)
  byte                   ramp;   // Rampa de velocidade. Incremento ou decremento em cada ms até chegar ao valor de velocidade desejada (0-255) 
  //byte                 StepMotorStepDiv;// Divisor de passos do driver do motor de passos
}; // mot; Arquivos de cabecalho nao devem conter definicoes de variaveis, somente declaracoes

#define CONV_SPEED_OCR 1309440
#define PASS_DIV 16
#define OCR_MIN 20


/* ----------------------------------------------------------------------------------------
 *  MEASUREMENTS
 *  daq.adcValue  : Array com valores em unidades de conversão do fluxo do ar (0), tensão elétrica na bateria (2), 
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
  volatile unsigned int adcValue[3];
  int           pres;
  int           flow;
  byte          vBat;
  int           temp;
  unsigned int  volCorr;
  unsigned int  volMinu;
  int           freq;
  int           presPla;
}; //daq; Arquivos de cabecalho nao devem conter definicoes de variaveis, somente declaracoes


/* ----------------------------------------------------------------------------------------
 *  RESPIRATION
 *  
 *  insSpeed: Velocidade do motor para inspiração
 *  insPos:   Posição final do curso inspiração
 *  expSpeed: Velocidade do motor para expiração
 *  insPos:   Posição final do curso expiração
 */
struct resp_type{
  unsigned int insSpe;
  unsigned int insPos;
  unsigned int expSpe;
  unsigned int expPos;
}; //resp; Arquivos de cabecalho nao devem conter definicoes de variaveis, somente declaracoes


/* ----------------------------------------------------------------------------------------
 *  COMMUNICATION PC
 *  
 */
 #define COM_PC_BR 115200  // Baus Rate
 #define SAMPLE_TIME 10 // 10 ms
 // volatile byte sendDat; Arquivos de cabecalho nao devem conter definicoes de variaveis, somente declaracoes


/* ----------------------------------------------------------------------------------------
 *  COMMUNICATION ESP-01
 *  
 */
 

/* ----------------------------------------------------------------------------------------
 *  LCD
 */




// volatile byte lcdFlag=0; Arquivos de cabecalho nao devem conter definicoes de variaveis, somente declaracoes

#define UPDATE_LCD_TIME 500


/* ----------------------------------------------------------------------------------------
 *  FUNCOES
 *  
 */


#define FIXED_POINT_FRACTIONAL_BITS 8

// Funcoes para inteiro 16-bits ponto fixo
float fixed_to_float(fixed_point_t fixed);

fixed_point_t float_to_fixed(float in);

// Escrita na EEPROM
void writeConfig(void *group, unsigned int id);

// Leitura na EEPROM
void readConfig(void *group, int id);

// Funcao para processamento de medidas
void processaMedidas();

// Som no buzzer
void sound(byte val);

// Funcao para processamento de alarmes
byte processaAlarmes();
                                    
#endif // _HEADERFILE_H

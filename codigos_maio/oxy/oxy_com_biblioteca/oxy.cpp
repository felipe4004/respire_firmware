// OXY_C
#include "oxy.h"
#include "oxy_types.h"
#include "avr/eeprom.h"

/*
 * Definicoes de variaveis.
 */
byte  HWSetup = 0x06;
struct motor_type mot;
struct daq_type daq;
struct resp_type resp;
volatile byte sendDat;
volatile byte lcdFlag=0;
volatile byte debounceCount=KEYB_DEBOUNCE;
volatile byte KeyLedState=0;

// ------------------------------------------------------------
// EEPROM Default values

const uint16_t EE_SMDiv=16;

valueFlag alarmFlag;

struct equipConfig_type   equipConfig;
struct respConfig_type    respConfig;
struct alarmConfig_type   alarmConfig;
struct alarmReg_type      alarmReg;
struct patientStats_type  patientStats;

/*
 * Definicao de funcoes.
 */

/*
   Le os dados da EEPROM.

   Recebe o ponteiro "group" para a struct em que os dados serao colocados.
   Recebe o "id" (offset da EEPROM) do tipo de informacao, baseado nas
   constantes definidas em "oxy_eeprom.h".
*/
void readConfig(void *group, int id) {  // ID vai ser o offset na EEPROM
  int sz;
  switch (id) {
    case EE_EQU:
      sz = sizeof(struct equipConfig_type);
      break;
    case EE_RES:
      sz = sizeof(struct respConfig_type);
      break;
    case EE_ALA:
      sz = sizeof(struct alarmConfig_type);
      break;
    case EE_RAL:
      sz = sizeof(struct alarmReg_type);
      break;
    case EE_STA:
      sz = sizeof(struct patientStats_type);
      break;
    default:
      return;
  }
  for (int i = 0; i < sz; i++) {
    *((uchar *)group + i) = eeprom_read_byte((uint8_t *) id + i);
  }
}

/*
   Escreve na EEPROM.

   Recebe o ponteiro "group" para a struct de onde a informação será lida
   Recebe o "id" (offset da EEPROM) do tipo de informacao, baseado nas
   constantes definidas em "oxy_eeprom.h".
*/
void writeConfig(void *group, unsigned int id) {
  int sz;
  switch (id) {
    case EE_EQU:
      sz = sizeof(struct equipConfig_type);
      break;
    case EE_RES:
      sz = sizeof(struct respConfig_type);
      break;
    case EE_ALA:
      sz = sizeof(struct alarmConfig_type);
      break;
    case EE_RAL:
      sz = sizeof(struct alarmReg_type);
      break;
    case EE_STA:
      sz = sizeof(struct patientStats_type);
      break;
    default:
      return;
  }
  for (int i = 0; i < sz; i++) {
    eeprom_write_byte((uint8_t *) id + i, *((uchar *)group + i));
  }
}

/*
   Converter de ponto fixo 16 bits para ponto flutuante 32 bits.

   Recebe o valor de ponto fixo e retorna o valor de ponto flutuante.
*/
float fixed_to_float(fixed_point_t fixed) {
  return (float)fixed / (1 << FIXED_POINT_FRACTIONAL_BITS);
}

/*
   Converter de ponto flutuante 32 bits para ponto fixo 16 bits.

   Recebe o valor de ponto flutuante e retorna o valor de ponto fixo.
*/
fixed_point_t float_to_fixed(float in) {
  fixed_point_t result = (unsigned int) in;
  in -= result;
  for (int i = 0; i < FIXED_POINT_FRACTIONAL_BITS; i++) {
    in *= 2;
  }
  return (result << FIXED_POINT_FRACTIONAL_BITS) + in;
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

/*
   Realizar o processamento de medidas.

   Atualiza as medidas realizadas pelo ADC postas na estrutura
   daq.adcValues, e atualiza os valores daq.pres, daq.flow, daq.vBat
   e daq.presPla.
*/
void processaMedidas() { // Insere na struct as medidas processadas, vindas do ADC
  // Ponto fixo em 8 bits.8 bits -> 16 bits

  //Pressao
  fixed_point_t pressure = 0;
  float pressure_float = 0, dp = 0, pressure_plateau_float = 0;
  float d2p = 0, d2p_min = 100; // Rever isso aqui, esse valor maximo da segunda derivada
  static float pressure_float_old = 0, pressure_const = 0, dp_old = 0;

  // Recebe o valor do ADC (daq.adcValue), converte pra tensao, adiciona a tensao
  // de vies do amp op e converte pra float de 32 bits
  pressure_float = (3.6 * daq.adcValue[2] * 5 / 1024.0 + 1.92307);
  pressure_float = pressure_float / (daq.adcValue[2] * equipConfig.press_gain) + equipConfig.press_offset / equipConfig.press_sens;
  pressure_float = pressure_float * 10.19716;  // Converte kPa para cmH2O
  daq.pres = float_to_fixed(pressure_float);


  // Pressao plato
  // A pressao plato sera a regiao de minimo da segunda derivada
  dp = pressure_float - pressure_float_old;  //Primeira derivada
  d2p = dp - dp_old;
  if (d2p < d2p_min) {
    d2p_min = d2p;
  }
  dp_old = dp;
  pressure_float_old = pressure_float;
  daq.presPla = float_to_fixed(d2p_min);
  

  // Fluxo de ar
  fixed_point_t flow = 0;
  float flow_float = 2 * daq.adcValue[0] * 5 / 1024;
  byte index = 0;

  if (flow_float > 8.2) {
    index = 0;
  } else if (flow_float > 5.8) {
    index = 1;
  } else if (flow_float > 4.1) {
    index = 2;
  } else if (flow_float > 1.7) {
    index = 3;
  } else {
    index = 4;
  }

  daq.flow = equipConfig.a[index] * float_to_fixed(flow_float) + equipConfig.b[index];  // Converte para l/min

  // Tensao da bateria
  float batt_float = 3 * daq.adcValue[2] * 5 / 1024;
  daq.vBat = float_to_fixed(batt_float);
}


/*
 * Processamento de alarmes
 * 
 * Deve ser chamada depois que as medidas forem feitas, para que se possa aferir
 * se elas passaram do limite ou nao. Caso alguma medida tenha passado do limite
 * estipulado na estrutura de configuracao de alarmes alarmConfig, é levantada a 
 * respectiva flag na estrutura de registro de alarmes alarmReg e é acionado o
 * buzzer por meio da funcao sound(). Para resetar alarme, deve-se usar a funcao
 * limpaAlarme().
 * 
 * Retorna se foi levantada uma flag de alarme (1) ou nao (0).
 */

byte processaAlarmes() {
  byte raiseAlarm = 0;

  if (daq.pres > alarmConfig.pres_max) {
    alarmReg.pres_exceed = 1;
    raiseAlarm = 1;
  } else if (daq.pres < alarmConfig.pres_min) {
    alarmReg.pres_insuf = 1;
    raiseAlarm = 1;
  }

  if (daq.volCorr > alarmConfig.vc_max) {
    alarmReg.vc_exceed = 1;
    raiseAlarm = 1;
  } else if (daq.volCorr < alarmConfig.vc_min) {
    alarmReg.vc_insuf = 1;
    raiseAlarm = 1;
  }

  if (daq.freq > alarmConfig.freq_max) {
    alarmReg.freq_exceed = 1;
    raiseAlarm = 1;
  } else if (daq.freq < alarmConfig.freq_min) {
    alarmReg.freq_insuf = 1;
    raiseAlarm = 1;
  }

  if (daq.vBat < alarmConfig.batt_min) {
    alarmReg.low_batt = 1;
    raiseAlarm = 1;
  }

  if(raiseAlarm)
    sound(21); // Aprox. 880 Hz
  
  return raiseAlarm;
}

/*
 * Limpa o alarme com maior prioridade.
 * 
 * Deve ser chamada depois que o usurario confirmou via teclado que o
 * alarme foi atendido.
 */

void limpaAlarme() {
  // limpar flag do alarme de maior prioridade que foi chamada
  uchar *ptr = (uchar *) &alarmReg;
  for(int i=0; i<sizeof(alarmReg); i++) {
    if(*ptr == 1) {
      *ptr = 0;
      break;
    }
    ptr++;
  }
  sound(0); // Reseta o buzzer
}


// OXY_C

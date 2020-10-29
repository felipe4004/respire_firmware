#ifndef _OXY_TYPES_H
#define _OXY_TYPES_H

#include <Arduino.h>
#include "oxy_eeprom.h"
//------- TYPEDEFS
/*
 "avr-gcc -fshort-enums" to compile enums below as the smallest integer size
 (16-bits in this case, and considering that further enums are limited to 16-bits
  as well)
*/

typedef enum { // EEPROM group offsets are defined here
    _equipConf = EE_EQU, // Configuracao do equipamento
    _respConf = EE_RES, // Configuracao da respiracao
    _alarmConf = EE_ALA, // Configuracao dos alarmes
    _alarmReg = EE_RAL, // Registro de alarmes
    _patientStats = EE_STA// Estatistica do paciente
} group_id;

typedef enum {  
  VC_INSUFF = 0x01,    // Volume corrente insuficiente
  PRES_INSUFF = 0x02,  // Pressao insuficiente
  VC_EXCEED = 0x04,    // Volume corrente excedente
  PRES_EXCEED = 0x08,  // Pressao excedente
  VE_INSUFF = 0x10,    // Volume minuto insuficiente
  VE_EXCEED = 0x20,    // Volume minuto excedente
  FREQ_INSUFF = 0x40,  // Frequencia insuficiente
  FREQ_EXCEED = 0x80   // Frequencia excedente
} valueFlag;

typedef unsigned char uchar;
//------END TYPEDEFS

valueFlag alarmFlag;

//------STRUCTS and UNIONS

#define GROUP_BLOCK_SIZE 40

struct equipConfig {
    uint16_t ID;            // ID do OxigStation
    uint16_t press_offset;  // Offset medicao pressao
    uint16_t press_gain;    // Ganho medicao pressao
    uint16_t press_on_min;  // Valor minimo para considerar o sensor de pressao desligado
    uint16_t flux_offset;   // Offset medicao fluxo
    uint16_t flux_gain;     // Ganho medicao fluxo
    uint16_t flux_off;      // Sensor de fluxo desligado
    uint16_t batt_offset;   // Offset medicao de % da bateria
    uint16_t batt_gain;     // Ganho medicao de % da bateria
    uchar batt_min;         // Valor minimo de % da bateria
    uchar ssid;             // Numero do OxiPanel (SSID WiFi) 
};

struct measurements {
  uint16_t vc;              // Volume corrente medido
  uint16_t flux;            // Fluxo
  uint16_t pressure;        // Pressao
  uint16_t battery_level;   // Nivel de tensao da bateria
  uint16_t freq;            // Frequencia respiratoria
};

struct respConfig {
    uchar idade;        // Idade
    uchar sexo;         // Sexo
    uchar modo_vent;    // Modos de ventilacao
    uchar modo_disp;    // Modo de disparo
    uint16_t vc;        // Volume corrente
    uchar press_ctrl;   // Pressao controlada
    uchar tipo_flux;    // Tipo de fluxo
    uchar peep;         // PEEP
    uchar fio2;         // FiO2
    uchar freq_resp;    // Frequencia respiratoria
    uint32_t flux;      // Fluxo calculado a partir de VC e rpm
    uchar IE;           // relacao I:E
};

struct alarmConfig { // Valores limite que disparam o alarme
  uint16_t vc_min;      // Volume corrente minimo
  uint16_t vc_max;      // Volume corrente maximo
  uchar pres_min;       // Pressao minima
  uchar pres_max;       // Pressao maxima
  uint16_t ve_min;
  uint16_t ve_max;     
  uchar freq_min;       // Frequencia minima
  uchar freq_max;       // Frequencia maxima
  uchar batt_min;
};

struct alarmReg {
  uchar pwr_supply_fail;  // Falha na fonte de alimentacao
  uchar off_ventilation;  // Ventilador desligado quando deveria estar ligado
  uchar pres_exceed;      // Pressao excedeu a pressao maxima
  uchar pres_insuf;       // Pressao insuficiente para alcancar a sensibilidade
  uchar freq_exceed;      // Frequencia excedeu valor maximo
  uchar freq_insuf;       // Frequencia nao alcancou valor minimo
  uchar vc_exceed;        // Volume corrente maximo excedido
  uchar vc_insuf;         // Volume corrente insuficiente (< minimo)
  uchar low_batt;         // Bateria baixa
};

struct patientStats {
  uint16_t vc_min;        // Volume corrente minimo
  uint16_t vc_avg;        // Volume corrente medio
  uint16_t vc_max;        // Volume corrente maximo
  uchar press_min;        // Pressao minima
  uchar press_avg;        // Pressao media
  uchar press_max;        // Pressao maxima
  uchar peep_min;         // PEEP minima
  uchar peep_avg;         // PEEP media
  uchar peep_max;         // PEEP maxima
  uchar plat_min;         // Pressao plato minima
  uchar plat_avg;         // Pressao plato media
  uchar plat_max;         // Pressao plato maxima
  uchar flux_min;         // Fluxo minimo
  uchar flux_avg;         // Fluxo medio
  uchar flux_max;         // Fluxo maximo
  uchar freq_min;         // Frequencia minima
  uchar freq_avg;         // Frequencia media
  uchar freq_max;         // Frequencia maxima
};

//------END STRUCTS and UNIONS

#endif // _OXY_TYPES_H
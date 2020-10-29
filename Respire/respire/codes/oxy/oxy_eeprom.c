#include <Arduino.h>
#include "oxy_types.h"
#include "oxy_eeprom.h"

/*
 * Le os dados da EEPROM
 * Recebe o ponteiro para a struct em que os dados serao colocados.
 * Recebe o id (offset da EEPROM) do tipo de informacao, baseado nas
 * constantes definidas em "oxy_eeprom.h".
 */

void readConfig(void *group, int id) { // ID vai ser o offset na EEPROM
    int sz;
    switch(id) {
        case EE_EQU:
            sz = sizeof(struct equipConfig);
            break;
        case EE_RES:
            sz = sizeof(struct respConfig);
            break;
        case EE_ALA:
            sz = sizeof(struct alarmConfig);
            break;
        case EE_RAL:
            sz = sizeof(struct alarmReg);
            break;
        case EE_STA:
            sz = sizeof(struct patientStats);
            break;
        default:
            return;
    }
    for(int i=0; i<sz; i++) {
        *((uchar *)group + i) = eeprom_read_byte((uint16_t *) id+i); 
    }
}

/*
 * Escrita na EEPROM.
 * Recebe o ponteiro para a stuct de onde a informação será lida
 * Recebe o id (offset da EEPROM) do tipo de informacao, baseado nas
 * constantes definidas em "oxy_eeprom.h".
 */

void writeConfig(void *group, group_id id) {
    int sz;
    switch(id) {
        case EE_EQU:
            sz = sizeof(struct equipConfig);
            break;
        case EE_RES:
            sz = sizeof(struct respConfig);
            break;
        case EE_ALA:
            sz = sizeof(struct alarmConfig);
            break;
        case EE_RAL:
            sz = sizeof(struct alarmReg);
            break;
        case EE_STA:
            sz = sizeof(struct patientStats);
            break;
        default:
            return;
    }
    for(int i=0; i<sz; i++) {
        eeprom_write_byte((uint16_t *) id+i, *((uchar *)group + i));
    }
}
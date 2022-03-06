/* 
 * File:   main.h
 * Author: user
 *
 * Created on March 5, 2022, 3:42 PM
 */

#ifndef MAIN_H
#define	MAIN_H

#include "mcc_generated_files/adc.h"


enum {
    SLEEP = 0,
    IDLE,
    CHARGING,
    CHARGING_WAIT,
    CHARGE_OC_LOCKOUT,
    CELL_BALANCE,
    OUTPUT_EN,
    DISCHARGE_OC_LOCKOUT,
} state;

enum{
    NONE = 0,
    TRIGGER,
    CHARGER
} detect;

typedef enum {
    SV09 = 0,
    SV11 = 1,
    NUM_OF_MODELS,
} modelnum_t;


int16_t volatile test;
uint16_t volatile isl_thermistor;
uint16_t volatile pic_thermistor;
double volatile thermistor_temp;
modelnum_t volatile modelnum;

uint16_t readADCmV(adc_channel_t channel);














#endif	/* MAIN_H */


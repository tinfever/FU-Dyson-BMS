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
    INIT = 0,
    SLEEP,
    IDLE,
    CHARGING,
    CHARGING_WAIT,
    CHARGE_OC_LOCKOUT,
    CELL_BALANCE,
    OUTPUT_EN,
    DISCHARGE_OC_LOCKOUT,
    ERROR,
} state;

typedef enum{
    NONE = 0,
    TRIGGER,
    CHARGER
} detect_t;

detect_t volatile detect;

typedef enum {
    SV09 = 0,
    SV11 = 1,
    NUM_OF_MODELS,
} modelnum_t;

modelnum_t volatile modelnum;

int16_t volatile isl_int_temp;
uint8_t volatile thermistor_temp;
uint16_t charge_wait_timer = 0;
bool charge_complete_flag = false;

typedef struct {
    uint16_t value;
    bool enable;
} counter_t;

typedef struct {
    uint32_t value;
    bool enable;
} big_counter_t;

counter_t charge_wait_counter = {0,0};
big_counter_t charge_duration_counter = {0,0};
counter_t sleep_timeout_counter = {0,0};

uint16_t readADCmV(adc_channel_t channel);














#endif	/* MAIN_H */


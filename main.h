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
    CELL_BALANCE,
    OUTPUT_EN,
    ERROR,
} state;

typedef enum{
    NONE = 0,
    TRIGGER,
    CHARGER
} detect_t;

detect_t detect = 0;
//#define DETECT_HISTORY_SIZE 4
//detect_t detect_history[DETECT_HISTORY_SIZE] = 0;
//uint8_t oldest_detect_index = 0;

uint8_t detect_history = 0; //Bits 0-1 = position 0; Bits 2-3 = position 1; Bits 4-5 = position 2; Bits 6-7 = position 3


typedef enum {
    SV09 = 0,
    SV11 = 1,
    NUM_OF_MODELS,
} modelnum_t;
modelnum_t modelnum;

int16_t isl_int_temp;
uint8_t thermistor_temp;
bool charge_complete_flag = false;
bool full_discharge_flag = false;
uint16_t discharge_current_mA = 0;

typedef struct {
    uint16_t value;
    bool enable;
} counter_t;

typedef struct {
    uint32_t value;
    bool enable;
} big_counter_t;

counter_t charge_wait_counter = {0,0};
counter_t sleep_timeout_counter = {0,0};
counter_t nonblocking_wait_counter = {0,0};
counter_t error_timeout_wait_counter = {0,0};
big_counter_t charge_duration_counter = {0,0};
counter_t LED_code_cycle_counter = {0,0};
detect_t GetDetectHistory(uint8_t position);
bool ChargerInDetectHistory(void);

uint16_t readADCmV(adc_channel_t channel);













#endif	/* MAIN_H */


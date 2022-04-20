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


const uint8_t EEPROM_NEXT_BYTE_AVAIL_STORAGE_ADDR = 0x18;
const uint8_t EEPROM_RUNTIME_TOTAL_STARTING_ADDR = 0x1C;    //32-bit runtime counter to be held in 0x1C, 0x1D, 0x1E, 0x1F
    


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
big_counter_t total_runtime_counter = {0,0};

detect_t GetDetectHistory(uint8_t position);
bool CheckStateInDetectHistory(detect_t detect_val);

uint16_t readADCmV(adc_channel_t channel);
void WriteTotalRuntimeCounterToEEPROM(uint8_t starting_addr);













#endif	/* MAIN_H */


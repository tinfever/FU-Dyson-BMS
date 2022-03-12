/* 
 * File:   main.h
 * Author: user
 *
 * Created on March 5, 2022, 3:42 PM
 */

#ifndef MAIN_H
#define	MAIN_H

#include "mcc_generated_files/adc.h"

#ifdef __INTELLISENSE__     //Make it so VSCode doesn't complain about what a "__bit" is.
#define __bit bool
#endif

enum {
    INIT = 0,
    SLEEP,
    IDLE,
    CHARGING,
    CHARGING_WAIT,
    //CHARGE_OC_LOCKOUT,
    CELL_BALANCE,
    OUTPUT_EN,
    //DISCHARGE_OC_LOCKOUT,
    ERROR,
    //IDLE_WAIT_TRIGGER_RELEASE,
} state;

typedef enum{
    NONE = 0,
    TRIGGER,
    CHARGER
} detect_t;

detect_t volatile detect;

typedef enum {
    NO_ERROR                                            = 0b0,    //0x0
    ISL_INT_OVERTEMP_FLAG                               = 0b1,    //0x1
    ISL_EXT_OVERTEMP_FLAG                              = 0b10,    //0x2
    ISL_INT_OVERTEMP_PICREAD                          = 0b100,    //0x4
    THERMISTOR_OVERTEMP_PICREAD                      = 0b1000,    //0x8
    CHARGE_OC_FLAG                                  = 0b10000,    //0x10
    DISCHARGE_OC_FLAG                              = 0b100000,    //0x20
    DISCHARGE_SC_FLAG                             = 0b1000000,    //0x40
    LOAD_PRESENT_FLAG                           = 0b10000000,    //0x80
    TRIG_PRESENT                               = 0b100000000,    //0x100
    CHRG_PRESENT                              = 0b1000000000,    //0x200
    DISCHARGE_OC_SHUNT_PICREAD               = 0b10000000000,    //0x400
    CHARGE_ISL_OVERTEMP_PICREAD             = 0b100000000000,    //0x800
    CHARGE_THERMISTOR_OVERTEMP_PICREAD     = 0b1000000000000,    //0x1000
    TEMP_HYSTERESIS                       = 0b10000000000000,    //0x2000
} error_reason_t;

volatile error_reason_t past_error_reason = 0;          //It's called past error reason because by the time you read it's data, the data will be the reason in the past for the error, regardless of the current state
volatile error_reason_t current_error_reason = 0;

typedef enum {
    SV09 = 0,
    SV11 = 1,
    NUM_OF_MODELS,
} modelnum_t;

modelnum_t volatile modelnum;

typedef enum{
    PAST = 0,
    PRESENT = 1,
} spacetime_position_t;

int16_t volatile isl_int_temp;
uint8_t volatile thermistor_temp;
uint16_t charge_wait_timer = 0;
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
big_counter_t charge_duration_counter = {0,0};
counter_t sleep_timeout_counter = {0,0};

uint16_t readADCmV(adc_channel_t channel);
void setErrorReasonFlags(volatile error_reason_t *datastore, spacetime_position_t timeframe);













#endif	/* MAIN_H */


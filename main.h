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

//typedef enum {
//    NO_ERROR                                            = 0b0,    //0x0         0b 00000000 00000000
//    ISL_INT_OVERTEMP_FLAG                               = 0b1,    //0x1         0b 00000000 00000001
//    ISL_EXT_OVERTEMP_FLAG                              = 0b10,    //0x2         0b 00000000 00000010
//    ISL_INT_OVERTEMP_PICREAD                          = 0b100,    //0x4         0b 00000000 00000100
//    THERMISTOR_OVERTEMP_PICREAD                      = 0b1000,    //0x8         0b 00000000 00001000
//    CHARGE_OC_FLAG                                  = 0b10000,    //0x10        0b 00000000 00010000
//    DISCHARGE_OC_FLAG                              = 0b100000,    //0x20        0b 00000000 00100000
//    DISCHARGE_SC_FLAG                             = 0b1000000,    //0x40        0b 00000000 01000000
//    LOAD_PRESENT_FLAG                           = 0b10000000,    //0x80         0b 00000000 10000000
//    TRIG_PRESENT                               = 0b100000000,    //0x100        0b 00000001 00000000
//    CHRG_PRESENT                              = 0b1000000000,    //0x200        0b 00000010 00000000
//    DISCHARGE_OC_SHUNT_PICREAD               = 0b10000000000,    //0x400        0b 00000100 00000000
//    CHARGE_ISL_OVERTEMP_PICREAD             = 0b100000000000,    //0x800        0b 00001000 00000000
//    CHARGE_THERMISTOR_OVERTEMP_PICREAD     = 0b1000000000000,    //0x1000       0b 00010000 00000000
//    TEMP_HYSTERESIS                       = 0b10000000000000,    //0x2000       0b 00100000 00000000
//} error_reason_t;

//volatile error_reason_t past_error_reason = 0;          //It's called past error reason because by the time you read it's data, the data will be the reason in the past for the error, regardless of the current state
//volatile error_reason_t current_error_reason = 0;

typedef struct {
    bool ISL_INT_OVERTEMP_FLAG : 1;
    bool ISL_EXT_OVERTEMP_FLAG : 1;
    bool ISL_INT_OVERTEMP_PICREAD : 1;
    bool THERMISTOR_OVERTEMP_PICREAD : 1;
    bool CHARGE_OC_FLAG : 1;
    bool DISCHARGE_OC_FLAG : 1;
    bool DISCHARGE_SC_FLAG : 1;
    bool LOAD_PRESENT_FLAG : 1;
    bool TRIG_PRESENT : 1;
    bool CHRG_PRESENT : 1;
    bool DISCHARGE_OC_SHUNT_PICREAD : 1;
    bool CHARGE_ISL_OVERTEMP_PICREAD : 1;
    bool CHARGE_THERMISTOR_OVERTEMP_PICREAD : 1;
    bool TEMP_HYSTERESIS : 1;
    detect_t DETECT_MODE;
} error_reason_t;

volatile error_reason_t past_error_reason = {0};          //It's called past error reason because by the time you read it's data, the data will be the reason in the past for the error, regardless of the current state
volatile error_reason_t current_error_reason = {0};


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


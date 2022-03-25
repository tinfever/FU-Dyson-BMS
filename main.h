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
detect_t volatile detect;

typedef struct {
    bool ISL_INT_OVERTEMP_FLAG : 1;
    bool ISL_EXT_OVERTEMP_FLAG : 1;
    bool ISL_INT_OVERTEMP_PICREAD : 1;
    bool THERMISTOR_OVERTEMP_PICREAD : 1;
    bool CHARGE_OC_FLAG : 1;
    bool DISCHARGE_OC_FLAG : 1;
    bool DISCHARGE_SC_FLAG : 1;
    bool DISCHARGE_OC_SHUNT_PICREAD : 1;
    bool CHARGE_ISL_INT_OVERTEMP_PICREAD : 1;
    bool CHARGE_THERMISTOR_OVERTEMP_PICREAD : 1;
    bool TEMP_HYSTERESIS : 1;
    bool ERROR_TIMEOUT_WAIT : 1;
    bool LED_BLINK_CODE_MIN_PRESENTATIONS : 1;
    detect_t DETECT_MODE;
} error_reason_t;

/*//It's called past error reason because by the time you read its data,
 *  the data will be the reason in the past for the error, regardless of the current state */
volatile error_reason_t past_error_reason = {0};          
volatile error_reason_t current_error_reason = {0};

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

uint16_t readADCmV(adc_channel_t channel);
void setErrorReasonFlags(volatile error_reason_t *datastore);
void Set_LED_RGB(uint8_t RGB, uint16_t PWM_val);













#endif	/* MAIN_H */


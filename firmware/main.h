/*
* FW-Dyson-BMS	-	(unofficial) Firmware Upgrade for Dyson BMS - V6/V7 Vacuums
* Copyright (C) 2022 tinfever
* 
* This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
* 
* The author can be contacted at tinfever6@(insert-everyone's-favorite-google-email-domain).com
* 
* NOTE: As an addendum to the GNU General Public License, any hardware using code or information from this project must also make publicly available complete electrical schematics and a bill of materials for such hardware.
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
    NONE = 0,       //0b00
    TRIGGER = 1,    //0b01
    CHARGER = 2     //0b10
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
uint8_t I2C_error_counter = 0;


    


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
void ClearI2CBus(void);













#endif	/* MAIN_H */


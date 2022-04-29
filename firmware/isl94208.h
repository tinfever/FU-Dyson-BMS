/*
* FU-Dyson-BMS	-	(unofficial) Firmware Upgrade for Dyson BMS - V6/V7 Vacuums
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

#ifndef ISL94208_H
#define	ISL94208_H
#include "mcc_generated_files/mcc.h"
#include "i2c.h"
#include "config.h"

typedef enum {
    Config = 0x00,
    Status,
    CellBalance,
    AnalogOut,
    FETControl,
    DischargeSet,
    ChargeSet,
    FeatureSet,
    WriteEnable, 
    __ISL_NUMBER_OF_REG
} isl_reg_t;

uint8_t ISL_RegData[__ISL_NUMBER_OF_REG] = {0};

i2c_result_t I2C_ERROR_FLAGS = 0;

uint16_t CellVoltages[7] = {0}; //Array for cell voltages. We'll just ignore index 0 and use indexes 1-6 for cells 1-6

#ifdef ENABLE_CELL_VOLTAGE_ROLLING_AVERAGE
uint16_t CellVoltageHistory[CELLVOLTAGE_AVERAGE_WINDOW_SIZE][7] = {0};
uint8_t OldestVoltageIndex = 0;
#endif

enum {
    REG_ADDRESS = 0,
    BIT_ADDRESS = 1,
    BIT_LENGTH = 2,
};

typedef uint8_t isl_locate_t;

const struct ISL_reg_bits_struct {  //Stores one array for each register setting in the format {register address, bit addr of LSB of value, bit length of value
    isl_locate_t WKUP_STATUS[3];    //0x00 Config Register
    isl_locate_t PRESENT[3];
    
    isl_locate_t OC_CHARGE_STATUS[3];      //0x01 Operating Status Register
    isl_locate_t OC_DISCHARGE_STATUS[3];
    isl_locate_t SHORT_CIRCUIT_STATUS[3];
    isl_locate_t LOAD_FAIL_STATUS[3];
    isl_locate_t INT_OVER_TEMP_STATUS[3];
    isl_locate_t EXT_OVER_TEMP_STATUS[3];
    
    isl_locate_t CELL_BALANCE_6bits[3]; //0x02 Cell Balance Registers
    
    isl_locate_t ANALOG_OUT_SELECT_4bits[3];  //0x03 Analog Out Register
    isl_locate_t USER_FLAG_0[3];   
    isl_locate_t USER_FLAG_1[3];
            
    isl_locate_t ENABLE_DISCHARGE_FET[3];   //0x04 FET Control Register
    isl_locate_t ENABLE_CHARGE_FET[3];
    isl_locate_t VMON_CHECK[3];
    isl_locate_t SLEEP[3];
            
    isl_locate_t OC_DISCHARGE_TIMEOUT_2bits[3];   //0x05 Discharge Set Register   
    isl_locate_t SC_DISCHARGE_THRESH_2bits[3];
    isl_locate_t SC_AUTO_DISABLE[3];
    isl_locate_t OC_DISCHARGE_THRESH_2bits[3];
    isl_locate_t OC_DISCHARGE_AUTO_DISABLE[3];
            
    isl_locate_t OC_CHARGE_TIMEOUT_2bits[3];  //0x06 Charge Set Register
    isl_locate_t DISCHARGE_TIME_DIV[3];
    isl_locate_t CHARGE_TIME_DIV[3];
    isl_locate_t SC_DELAY_LONG[3];
    isl_locate_t OC_CHARGE_THRESH_2bits[3];
    isl_locate_t OC_CHARGE_AUTO_DISABLE[3];
            
    isl_locate_t WKPOL[3];          //0x07 Feature Set Register
    isl_locate_t DISABLE_WKUP[3];
    isl_locate_t FORCE_POR[3];
    isl_locate_t DISABLE_INT_THERMAL_SHUTDOWN[3];
    isl_locate_t DISABLE_EXT_THERMAL_SHUTDOWN[3];
    isl_locate_t TEMP_3V_ON[3];
    isl_locate_t DISABLE_3V3_REG[3];
    isl_locate_t DISABLE_AUTO_TEMP_SCAN[3];
            
    isl_locate_t USER_FLAG_2[3];   //0x08 Write Enable Register
    isl_locate_t USER_FLAG_3[3];
    isl_locate_t ENABLE_DISCHARGE_SET_WRITES[3];
    isl_locate_t ENABLE_CHARGE_SET_WRITES[3];
    isl_locate_t ENABLE_FEAT_SET_WRITES[3];
} ISL = {
    .WKUP_STATUS = {0x00, 4, 1},    //0x00 Config Register; Register 0x00, LSB bit 4, bit length 1
    .PRESENT = {0x00, 5, 1},

    .OC_CHARGE_STATUS = {0x01, 0, 1},      //0x01 Operating Status Register
    .OC_DISCHARGE_STATUS = {0x01, 1, 1},
    .SHORT_CIRCUIT_STATUS = {0x01, 2, 1},
    .LOAD_FAIL_STATUS = {0x01, 3, 1},
    .INT_OVER_TEMP_STATUS = {0x01, 4, 1},
    .EXT_OVER_TEMP_STATUS = {0x01, 5, 1},

    .CELL_BALANCE_6bits = {0x02, 1, 6}, //0x02 Cell Balance Registers; Register 0x02, LSB in position 1, bit length 6
    
    .ANALOG_OUT_SELECT_4bits = {0x03, 0, 4},  //0x03 Analog Out Register, value requires 4 bits
    .USER_FLAG_0 = {0x03, 6, 1},   
    .USER_FLAG_1 = {0x03, 7, 1},

    .ENABLE_DISCHARGE_FET = {0x04, 0, 1},   //0x04 FET Control Register
    .ENABLE_CHARGE_FET = {0x04, 1, 1},
    .VMON_CHECK = {0x04, 6, 1},
    .SLEEP = {0x04, 7, 1},

    .OC_DISCHARGE_TIMEOUT_2bits = {0x05, 0, 2},   //0x05 Discharge Set Register, value requires 2 bits     
    .SC_DISCHARGE_THRESH_2bits = {0x05, 2, 2},
    .SC_AUTO_DISABLE = {0x05, 4, 1},
    .OC_DISCHARGE_THRESH_2bits = {0x05, 5, 2},
    .OC_DISCHARGE_AUTO_DISABLE = {0x05, 7, 1},

    .OC_CHARGE_TIMEOUT_2bits = {0x06, 0, 2},  //0x06 Charge Set Register, value requires 2 bits
    .DISCHARGE_TIME_DIV = {0x06, 2, 1},
    .CHARGE_TIME_DIV = {0x06, 3, 1},
    .SC_DELAY_LONG  = {0x06, 4, 1},
    .OC_CHARGE_THRESH_2bits = {0x06, 5, 2},
    .OC_CHARGE_AUTO_DISABLE = {0x06, 7, 1},

    .WKPOL = {0x07, 0, 1},          //0x07 Feature Set Register
    .DISABLE_WKUP = {0x07, 1, 1},
    .FORCE_POR = {0x07, 2, 1},
    .DISABLE_INT_THERMAL_SHUTDOWN = {0x07, 3, 1},
    .DISABLE_EXT_THERMAL_SHUTDOWN = {0x07, 4, 1},
    .TEMP_3V_ON = {0x07, 5, 1},
    .DISABLE_3V3_REG = {0x07, 6, 1},
    .DISABLE_AUTO_TEMP_SCAN = {0x07, 7, 1},

    .USER_FLAG_2 = {0x08, 3, 1},   //0x08 Write Enable Register
    .USER_FLAG_3 = {0x08, 4, 1},
    .ENABLE_DISCHARGE_SET_WRITES = {0x08, 5, 1},
    .ENABLE_CHARGE_SET_WRITES = {0x08, 6, 1},
    .ENABLE_FEAT_SET_WRITES = {0x08, 7, 1},
};

typedef enum {
    CB1 = 0b000001,
    CB2 = 0b000010,
    CB3 = 0b000100,
    CB4 = 0b001000,
    CB5 = 0b010000,
    CB6 = 0b100000,
} isl_cb_t;

typedef enum {
    AO_OFF =        0b0000,
    AO_VCELL1 =     0b0001,
    AO_VCELL2 =     0b0010,
    AO_VCELL3 =     0b0011,
    AO_VCELL4 =     0b0100,
    AO_VCELL5 =     0b0101,
    AO_VCELL6 =     0b0110,
    AO_EXTTEMP =    0b1000,
    AO_INTTEMP =    0b1001,
} isl_analogout_t;

struct {
    uint8_t mincellnum;     //Cell number with the lowest voltage
    uint8_t maxcellnum;     //Cell number with the highest voltage
    uint16_t maxcell_mV;    //Voltage of highest voltage cell in mV
    uint16_t mincell_mV;    //Voltage of lowest voltage cell in mV
    uint16_t packdelta_mV;  //mV difference between high and lowest voltage cells
    
} cellstats;

void ISL_Init(void);
uint8_t ISL_Read_Register(isl_reg_t reg);
void ISL_Write_Register(isl_reg_t reg, uint8_t wrdata);
void ISL_SetSpecificBits(const isl_locate_t params[3], uint8_t value);
uint8_t ISL_GetSpecificBits(const isl_locate_t params[3]);
uint8_t ISL_GetSpecificBits_cached(const isl_locate_t params[3]);
uint16_t ISL_GetAnalogOutmV(isl_analogout_t value);
void ISL_ReadAllCellVoltages(void);
int16_t ISL_GetInternalTemp(void);
void ISL_calcCellStats(void);
































#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* ISL94208_H */


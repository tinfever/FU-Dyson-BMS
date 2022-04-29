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

#include "FaultHandling.h"
#include "config.h"
#include "isl94208.h"


bool safetyChecks (void){
    bool result = true;
    result &= (isl_int_temp < MAX_DISCHARGE_TEMP_C);        //Internal ISL temp is OK
    result &= (thermistor_temp < MAX_DISCHARGE_TEMP_C);    //Thermistor temp is OK
    result &= (isl_int_temp > MIN_TEMP_C);                  //Make sure it isn't too cold for charger or discharging
    result &= (thermistor_temp > MIN_TEMP_C);  
    result &= (ISL_RegData[Status] == 0);               //No ISL error flags
    result &= (discharge_current_mA < MAX_DISCHARGE_CURRENT_mA);     //We aren't discharging more than 30A
    
    if (!result && state != ERROR){         //Makes sure we don't write new errors in to past_error_reason while we are in the error state
        setErrorReasonFlags(&past_error_reason);
    }
    
    return result;
}

bool minCellOK(void){
    return (cellstats.mincell_mV > MIN_DISCHARGE_CELL_VOLTAGE_mV);
}

bool maxCellOK(void){
    return (cellstats.maxcell_mV < MAX_CHARGE_CELL_VOLTAGE_mV);
}

bool chargeTempCheck(void){
    bool result = true;
    result &= (isl_int_temp < MAX_CHARGE_TEMP_C);
    result &= (thermistor_temp < MAX_CHARGE_TEMP_C);
    
    if (!result && state != ERROR){
        setErrorReasonFlags(&past_error_reason);
    }
    return result;
}


/* Most of the time the result of this function call will be stored in past_error_reason so that once we are in the actual error state, we still have a record of why we got there.
 * Once we are in the error state, we will repeatedly clear current_error_reason and store the result of this function in it so we can see the actual reason we haven't left the error state yet.
 * We were previously assuming that if we entered the error state and saw that the charger was connected and we were over the charge temp limit, that must be the reason for entry.
 * Since we are actually recording the reason for an error when it occurs, before any resolution has been taken,
 *  we can check that actual data to determine if we should be using the stricter charging temp limits.
 */
void setErrorReasonFlags(volatile error_reason_t *datastore){
    datastore->ISL_INT_OVERTEMP_FLAG |= ISL_GetSpecificBits_cached(ISL.INT_OVER_TEMP_STATUS);
    datastore->ISL_EXT_OVERTEMP_FLAG |= ISL_GetSpecificBits_cached(ISL.EXT_OVER_TEMP_STATUS);
    datastore->ISL_INT_OVERTEMP_PICREAD |= !(isl_int_temp < MAX_DISCHARGE_TEMP_C);
    datastore->THERMISTOR_OVERTEMP_PICREAD |= !(thermistor_temp < MAX_DISCHARGE_TEMP_C);
    datastore->UNDERTEMP_FLAG |= !(isl_int_temp > MIN_TEMP_C);
    datastore->UNDERTEMP_FLAG |= !(thermistor_temp > MIN_TEMP_C);
    datastore->CHARGE_OC_FLAG |= ISL_GetSpecificBits_cached(ISL.OC_CHARGE_STATUS);
    datastore->DISCHARGE_OC_FLAG |= ISL_GetSpecificBits_cached(ISL.OC_DISCHARGE_STATUS);
    datastore->DISCHARGE_SC_FLAG |= ISL_GetSpecificBits_cached(ISL.SHORT_CIRCUIT_STATUS);
    datastore->DISCHARGE_OC_SHUNT_PICREAD |= !(discharge_current_mA < MAX_DISCHARGE_CURRENT_mA);
    datastore->CHARGE_ISL_INT_OVERTEMP_PICREAD |= (state == CHARGING && !(isl_int_temp < MAX_CHARGE_TEMP_C));
    datastore->CHARGE_THERMISTOR_OVERTEMP_PICREAD |= (state == CHARGING && !(thermistor_temp < MAX_CHARGE_TEMP_C));
    datastore->ERROR_TIMEOUT_WAIT |= (state == ERROR && error_timeout_wait_counter.enable && !(error_timeout_wait_counter.value > ERROR_EXIT_TIMEOUT));
    datastore->LED_BLINK_CODE_MIN_PRESENTATIONS |= (state == ERROR && LED_code_cycle_counter.enable && !(LED_code_cycle_counter.value > NUM_OF_LED_CODES_AFTER_FAULT_CLEAR));
                    
    datastore->DETECT_MODE = detect;
    

    if (state == ERROR && 
            (  past_error_reason.ISL_INT_OVERTEMP_FLAG                  //Only stay in hysteresis lockout if the original fault was due to over-temp.
            || past_error_reason.ISL_EXT_OVERTEMP_FLAG
            || past_error_reason.ISL_INT_OVERTEMP_PICREAD
            || past_error_reason.THERMISTOR_OVERTEMP_PICREAD
            || past_error_reason.CHARGE_ISL_INT_OVERTEMP_PICREAD
            || past_error_reason.CHARGE_THERMISTOR_OVERTEMP_PICREAD
            || past_error_reason.UNDERTEMP_FLAG)
            ) { //If we are in the error state, we need to check if we are in hysteresis violation 
        datastore->TEMP_HYSTERESIS |= (isl_int_temp < MAX_DISCHARGE_TEMP_C && !(isl_int_temp + HYSTERESIS_TEMP_C < MAX_DISCHARGE_TEMP_C));  //Hysteresis only matters if we aren't over the main temp. limit.
        
        
        datastore->TEMP_HYSTERESIS |= (thermistor_temp < MAX_DISCHARGE_TEMP_C && !(thermistor_temp + HYSTERESIS_TEMP_C < MAX_DISCHARGE_TEMP_C));

        datastore->TEMP_HYSTERESIS |= (past_error_reason.DETECT_MODE == CHARGER                     //The past error occurred while on the charger
                                    && isl_int_temp < MAX_CHARGE_TEMP_C                             //but we aren't violating the main charge temp limit
                                    && !(isl_int_temp + HYSTERESIS_TEMP_C < MAX_CHARGE_TEMP_C));    //but we ARE still within the hysteresis range

        datastore->TEMP_HYSTERESIS |= (past_error_reason.DETECT_MODE == CHARGER
                                    && thermistor_temp < MAX_CHARGE_TEMP_C
                                    && !(thermistor_temp + HYSTERESIS_TEMP_C < MAX_CHARGE_TEMP_C));
        
        datastore->CHARGE_THERMISTOR_OVERTEMP_PICREAD |= (past_error_reason.DETECT_MODE == CHARGER  //The past error occurred while on the charger
                                                        && !(thermistor_temp < MAX_CHARGE_TEMP_C)); //and we are still violating the main charge temp limit
        
        datastore->CHARGE_ISL_INT_OVERTEMP_PICREAD |= (past_error_reason.DETECT_MODE == CHARGER  //The past error occurred while on the charger
                                                        && !(isl_int_temp < MAX_CHARGE_TEMP_C)); //and we are still violating the main charge temp limit
        
        datastore->TEMP_HYSTERESIS |= (thermistor_temp > MIN_TEMP_C) && !(thermistor_temp - HYSTERESIS_TEMP_C > MIN_TEMP_C); 
        datastore->TEMP_HYSTERESIS |= (isl_int_temp > MIN_TEMP_C) && !(isl_int_temp - HYSTERESIS_TEMP_C > MIN_TEMP_C); 

    }
    
}
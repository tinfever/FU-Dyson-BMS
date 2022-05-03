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

#ifndef FAULTHANDLING_H
#define	FAULTHANDLING_H

#include <stdbool.h>
#include "main.h"

typedef struct {
    bool ISL_INT_OVERTEMP_FLAG : 1;
    bool ISL_EXT_OVERTEMP_FLAG : 1;
    bool ISL_INT_OVERTEMP_PICREAD : 1;
    bool THERMISTOR_OVERTEMP_PICREAD : 1;
    bool UNDERTEMP_FLAG : 1;
    bool CHARGE_OC_FLAG : 1;
    bool DISCHARGE_OC_FLAG : 1;
    bool DISCHARGE_SC_FLAG : 1;
    bool DISCHARGE_OC_SHUNT_PICREAD : 1;
    bool CHARGE_ISL_INT_OVERTEMP_PICREAD : 1;
    bool CHARGE_THERMISTOR_OVERTEMP_PICREAD : 1;
    bool TEMP_HYSTERESIS : 1;
    bool ERROR_TIMEOUT_WAIT : 1;
    bool LED_BLINK_CODE_MIN_PRESENTATIONS : 1;
    bool ISL_BROWN_OUT : 1;
    detect_t DETECT_MODE;
} error_reason_t;

/*//It's called past error reason because by the time you read its data,
 *  the data will be the reason in the past for the error, regardless of the current state */
volatile error_reason_t past_error_reason = {0};          
volatile error_reason_t current_error_reason = {0};

bool safetyChecks (void);
bool minCellOK(void);
bool maxCellOK(void);
bool chargeTempCheck(void);
void setErrorReasonFlags(volatile error_reason_t *datastore);

























#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* FAULTHANDLING_H */


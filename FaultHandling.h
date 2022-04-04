/* 
 * File:   FaultHandling.h
 * Author: user
 *
 * Created on March 30, 2022, 3:05 PM
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


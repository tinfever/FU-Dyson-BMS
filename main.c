/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.7
        Device            :  PIC16LF1847
        Driver Version    :  2.00
*/

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/




#include "mcc_generated_files/mcc.h"
#include "main.h"
#include "i2c.h"
#include "isl94208.h"
#include "config.h"
#include "thermistor.h"
#include "led_blink_pattern.h"

void Set_LED_RGB(uint8_t RGB_en, uint16_t PWM_val){  //Accepts binary input 0b000. Bit 2 = Red Enable. Bit 1 = Green Enable. Bit 0 = Red Enable. R.G.B.
    
    EPWM1_LoadDutyValue(PWM_val);
    
    if (RGB_en & 0b001){
        blueLED = 1;
    }
    else{
        blueLED = 0;
    }
    
    if (RGB_en & 0b010){
        greenLED = 1;
    }
    else{
        greenLED = 0;
    }
    
    if (RGB_en & 0b100){
        redLED = 1;
    }
    else{
        redLED = 0;
    }
}

void ClearI2CBus(){
    uint8_t initialState[] = {TRIS_SDA, TRIS_SCL, ANS_SDA, ANS_SCL, SDA, SCL, SSP1CON1bits.SSPEN}; //Backup initial pin setup state
    SSP1CON1bits.SSPEN = 0;     //Disable MSSP if enabled
    TRIS_SDA = 1;     //SDA - Set Data as input
    TRIS_SCL = 1;     //SCL - Set Clock as input = high-impedance output
    ANS_SDA = 0;      //Set both pins as digital
    ANS_SCL = 0;
    SCL = 0;    //Set SCL PORT to be low, then we'll toggle TRIS to switch between low and high-impedance
    
    uint8_t validOnes = 0;
    
    //Clock the SCL pin until we get 10 valid ones in a row like we'd expect from an idle bus
    while (validOnes < 10){
        TRIS_SCL = 0; //Clock low
        __delay_us(5); //Wait out clock low period
        TRIS_SCL = 1; //Clock high
        __delay_us(2.5);  //Wait until we are in the mid point of clock high period
        if (SDA == 1){  //Read data and check if SDA is high (idle)
            validOnes++;
        }
        else{
            validOnes = 0;  //if the data isn't a one, reset the counter so we get 10 in a row
        }
        __delay_us(2.5); //Wait remainder of clock high period
    }
    TRIS_SDA = (__bit) initialState[0];   //Restore initial pin I/O state
    TRIS_SCL = (__bit) initialState[1];
    ANS_SDA = (__bit) initialState[2];
    ANS_SCL = (__bit) initialState[3];
    SDA = (__bit) initialState[4];
    SCL = (__bit) initialState[5];
    SSP1CON1bits.SSPEN = (__bit) initialState[6];
    if (initialState[6]){
        ISL_Init();
    }
    I2C_ERROR_FLAGS = 0;
}

uint16_t static ConvertADCtoMV(uint16_t adcval){                //I included this function here and in isl94208 so that it could stand alone if needed. There is probably a better way to do this.
    return (uint16_t) ((uint32_t)adcval * VREF_VOLTAGE_mV / 1024);
}

void ADCPrepare(void){
    DAC_SetOutput(0);   //Make sure DAC is set to 0V
    ADC_SelectChannel(ADC_PIC_DAC); //Connect ADC to 0V to empty internal ADC sample/hold capacitor
    __delay_us(1);  //Wait a little bit
}

uint16_t readADCmV(adc_channel_t channel){        //Adds routine to switch to DAC VSS output to clear sample/hold capacitor before taking real reading
    ADCPrepare();
    return ConvertADCtoMV( ADC_GetConversion(channel) ); //Finally run the conversion and store the result
}

uint16_t dischargeIsense_mA(void){
    ADCPrepare();
    uint16_t adcval = ADC_GetConversion(ADC_DISCHARGE_ISENSE);
    return (uint16_t) ((uint32_t)adcval * VREF_VOLTAGE_mV * 1000 / 1024 / 2);  //This better maintains precision by doing the multiplication for 2500mV VREF and 1000mA/A in one step as a uint32_t. Then we divide by 1024 ADC steps and the 2mOhm shunt resistor.
}

detect_t checkDetect(void){
    uint16_t result = readADCmV(ADC_CHRG_TRIG_DETECT);
    if (result > DETECT_CHARGER_THRESH_mV){
        return CHARGER;
    }
    else if (result < DETECT_CHARGER_THRESH_mV && result > DETECT_TRIGGER_THRESH_mV){
        return TRIGGER;
    }
    else{
        return NONE;
    }
}

modelnum_t checkModelNum (void){
/* This function assumes that if the reading of the thermistor from the ISL is
 * > 100mV above the reading from the PIC, we must be using an SV09 board 
 * which has an opamp driving the ISL thermistor input to ~3.3V until the thermistor voltage
 * goes below ~820mV, then it drives the ISL input to ~0V.
 * The SV11 has the thermistor input to the ISL and PIC tied together since
 * there is just the thermistor with one pull-up resistor. It uses a different voltage scale though. */
    uint16_t isl_thermistor_reading = ISL_GetAnalogOutmV(AO_EXTTEMP);
    uint16_t pic_thermistor_reading = readADCmV(ADC_THERMISTOR);
    int16_t delta = (int16_t)isl_thermistor_reading - (int16_t)pic_thermistor_reading;
    if (delta > 100){
        return SV09;
    }
    else{
        return SV11;
    }
}

bool safetyChecks (void){
    bool result = true;
    result &= (isl_int_temp < MAX_DISCHARGE_TEMP_C);        //Internal ISL temp is OK
    result &= (thermistor_temp < MAX_DISCHARGE_TEMP_C);    //Thermistor temp is OK
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
    datastore->CHARGE_OC_FLAG |= ISL_GetSpecificBits_cached(ISL.OC_CHARGE_STATUS);
    datastore->DISCHARGE_OC_FLAG |= ISL_GetSpecificBits_cached(ISL.OC_DISCHARGE_STATUS);
    datastore->DISCHARGE_SC_FLAG |= ISL_GetSpecificBits_cached(ISL.SHORT_CIRCUIT_STATUS);
    datastore->DISCHARGE_OC_SHUNT_PICREAD |= !(discharge_current_mA < MAX_DISCHARGE_CURRENT_mA);
    datastore->CHARGE_ISL_INT_OVERTEMP_PICREAD |= (state == CHARGING && !(isl_int_temp < MAX_CHARGE_TEMP_C));
    datastore->CHARGE_THERMISTOR_OVERTEMP_PICREAD |= (state == CHARGING && !(thermistor_temp < MAX_CHARGE_TEMP_C));
    datastore->ERROR_TIMEOUT_WAIT |= (state == ERROR && !(error_timeout_wait_counter.enable && error_timeout_wait_counter.value > ERROR_EXIT_TIMEOUT));
    datastore->LED_BLINK_CODE_MIN_PRESENTATIONS |= (state == ERROR && !(LED_code_cycle_counter.enable && LED_code_cycle_counter.value > NUM_OF_LED_CODES_AFTER_FAULT_CLEAR));
                    
    datastore->DETECT_MODE = detect;
    

    if (state == ERROR && 
            (  past_error_reason.ISL_INT_OVERTEMP_FLAG                  //Only stay in hysteresis lockout if the original fault was due to over-temp.
            || past_error_reason.ISL_EXT_OVERTEMP_FLAG
            || past_error_reason.ISL_INT_OVERTEMP_PICREAD
            || past_error_reason.THERMISTOR_OVERTEMP_PICREAD
            || past_error_reason.CHARGE_ISL_INT_OVERTEMP_PICREAD
            || past_error_reason.CHARGE_THERMISTOR_OVERTEMP_PICREAD)
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

    }
    
}

////////////////////////////////////////////////////////////////////



void init(void){
//INIT STEPS
    
    I2C_ERROR_FLAGS = 0;
    
    /* Initialize the device */
    SYSTEM_Initialize();
    TMR4_StartTimer();   //Keep timer running
    DAC_SetOutput(0);   //Make sure DAC output is 0V = VSS
    
    /* Initialize LEDS */
//    redLED = 1;  //Set high to default RED led to off
//    greenLED = 1;  //Set high to default GREEN led to off
//    blueLED = 1;  //Set high to default BLUE led to off
//    TRISB3 = 0; // Set greenLED as output
//    TRISA6 = 0; //Set blueLED as output
//    TRISA7 = 0; //Set redLED as output
//    ANSB3 = 0; //Set Green LED as digital. Red and Blue are always digital.

    /* Immediately start yellow LED so we know board is alive */
    //Set_LED_RGB(0b110);

    //* Set up I2C pins */
    TRIS_SDA = 1;     //SDA - Make sure both pins are inputs
    TRIS_SCL = 1;     //SCL
    ANS_SDA = 0;    //Set both pins as digital
    ANS_SCL = 0;
    I2C1_Init();
    ClearI2CBus();  //Clear I2C bus once on startup just in case
    while (SDA == 0 || SCL == 0){   //If bus is still not idle (meaning pins aren't high), which shouldn't be possible, then keep trying to clear bus. Do not pass go. Do not collect $200.
        __debug_break();
        ClearI2CBus();
    }
    
    ISL_SetSpecificBits(ISL.ENABLE_DISCHARGE_FET, 0);       //Make sure the pack is turned off in case we had some weird reset
    ISL_SetSpecificBits(ISL.ENABLE_CHARGE_FET, 0);
    modelnum = checkModelNum();
    //INIT END
    
    state = IDLE;
}

void sleep(void){
    ISL_SetSpecificBits(ISL.SLEEP, 1);
    __delay_us(50);
    ISL_SetSpecificBits(ISL.SLEEP, 0);
    __delay_us(50);
    ISL_SetSpecificBits(ISL.SLEEP, 1);

}

void idle(void){
    if (detect == TRIGGER                       //Trigger is pulled
        && minCellOK()          //Min cell is not below low voltage cut out of 3V
        && ISL_GetSpecificBits_cached(ISL.WKUP_STATUS) //Make sure WKUP = 1 meaning charger connected or trigger pressed
        && full_discharge_flag == false             //Make sure pack hasn't just been fully discharged
        && safetyChecks()
        ){
            state = OUTPUT_EN;
    }
    else if (detect == TRIGGER
        && full_discharge_flag == true
        ){
            state = ERROR;
    }
    else if (detect == CHARGER                       //Charger is connected
            && charge_complete_flag == false         //We haven't already done a complete charge cycle
            && maxCellOK()          //Max cell < 4.20V
            && ISL_GetSpecificBits_cached(ISL.WKUP_STATUS) //Make sure WKUP = 1 meaning charger connected or trigger pressed
            && safetyChecks()
        ){
                state = CHARGING; 
    }
    else if (detect == NONE                         //Start sleep counter if we are idle with no charger or trigger, but no errors
            && ISL_GetSpecificBits_cached(ISL.WKUP_STATUS) == 0
            && sleep_timeout_counter.enable == false
            && safetyChecks()
            ){
                sleep_timeout_counter.value = 0;        //Clear and start sleep counter
                sleep_timeout_counter.enable = true;
    }
    else if (!safetyChecks()){                        //Somehow there was an error  
        state = ERROR;
    }
    else if (detect == CHARGER                  //Set charge_complete_flag if pack is put on charger but max cell is already at maximum voltage
            && charge_complete_flag == false
            && !maxCellOK()
            ){
                charge_complete_flag = true;
            }
    else if (detect == CHARGER && charge_complete_flag){    //Pack on charger and charge is complete = Green LED
        Set_LED_RGB(0b010, 1023); //LED Green
    }
    else if (detect == CHARGER && ISL_GetSpecificBits_cached(ISL.WKUP_STATUS)) {                           //Pack on charger but charge isn't complete and we aren't in a charging or error state = Yellow LED, undefined state
        Set_LED_RGB(0b110, 1023); //LED Yellow
    }
    else if (detect == NONE){                               //No trigger or charger, pack is awake and idle = Green LED
        Set_LED_RGB(0b010, 1023); //LED Green
    }
    else if (detect == TRIGGER && ISL_GetSpecificBits_cached(ISL.WKUP_STATUS) && !full_discharge_flag){    //trigger is pulled but we didn't enable output, and it isn't because there was an error or the pack is fully discharged = Yellow LED
            //There is a delay inside ISL IC between when detect == TRIGGER and when WKUP == 1. adding && WKUP_STATUS make sure both are in the same state to avoid brief yellow LED flash.
        Set_LED_RGB(0b110, 1023); //LED Yellow    
    }
    
    //There is no handling for WKUP_STATUS and DETECT to be in different states. If WKUP_STATUS == 0 (don't wakeup) but DETECT == TRIGGER, God help us all.
    
    
    if ( (charge_complete_flag == true && cellstats.maxcell_mV < PACK_CHARGE_NOT_COMPLETE_THRESH_mV) || (detect != CHARGER)){      //If the max cell voltage is below 4100mV and the pack is marked as fully charged, unmark it as charged. Also, if removed from charger, clear charge complete flag.
        charge_complete_flag = false;
    }
    
    if (full_discharge_flag == false && !minCellOK() && detect != CHARGER){         //If min cell voltage is too low and we aren't on the charger, set full_discharge_flag so we can reference it for LED codes.
        full_discharge_flag = true;
    }
    
    if (sleep_timeout_counter.value >  IDLE_SLEEP_TIMEOUT && sleep_timeout_counter.enable == true){    //938*32ms = 30.016s //If we are in IDLE state for 30 seconds and not on the charger, go to sleep. We will stay awake on the charger since we have power to spare and can then make sure battery voltages don't drop over time.
        sleep_timeout_counter.enable = false;
        sleep_timeout_counter.value = 0;
        state = SLEEP;
    }
    

    //Clean up before going to different state
    if (state != IDLE) {
        sleep_timeout_counter.enable = false; //We aren't going to be sleeping soon
        resetLEDBlinkPattern();
    }
    
    
    //TODO: add entry in to error state if trigger pulled while voltage too low so we go to sleep
            //add entry to error state if charger connected while voltage too high?
    
}

void charging(void){

    if (!ISL_GetSpecificBits_cached(ISL.ENABLE_CHARGE_FET)     //if we aren't already charging
        && detect == CHARGER
        && maxCellOK()
        && ISL_GetSpecificBits_cached(ISL.WKUP_STATUS)
        && safetyChecks()
        && chargeTempCheck()
        ){
            charge_duration_counter.value = 0;
            charge_duration_counter.enable = true;          //Start charge timer
            ISL_SetSpecificBits(ISL.ENABLE_CHARGE_FET, 1);  //Enable Charging
            full_discharge_flag = false;                    //Clear full discharge flag once we start charging
            Set_LED_RGB(0b001, 1023); //Blue LED
    }
    else if (ISL_GetSpecificBits_cached(ISL.ENABLE_CHARGE_FET)     //same as above but we are already charging and all conditions are good
        && detect == CHARGER
        && maxCellOK()
        && ISL_GetSpecificBits_cached(ISL.WKUP_STATUS)
        && safetyChecks()
        && chargeTempCheck()
            ){
        //do nothing
    }
    else if (!maxCellOK()){         //Target voltage reached
        ISL_SetSpecificBits(ISL.ENABLE_CHARGE_FET, 0); //Disable Charging
        charge_duration_counter.enable = false;         //Stop charge timer
        if (charge_duration_counter.value < CHARGE_COMPELTE_TIMEOUT){   //313 * 32ms = 10.016s, if it took less than 10 seconds for max cell voltage to be > 4.20v, mark charge complete
            charge_complete_flag = true;
            state = IDLE;
        }
        else{       //Go to charge wait state and wait 70 seconds before starting next charge cycle
            state = CHARGING_WAIT;
        }
    }
    else if (!safetyChecks() || !chargeTempCheck()){     //There was an error
        ISL_SetSpecificBits(ISL.ENABLE_CHARGE_FET, 0); //Disable Charging
        charge_duration_counter.enable = false;         //Stop charge timer
        state = ERROR;
    }
    else{                                   //charger removed
        ISL_SetSpecificBits(ISL.ENABLE_CHARGE_FET, 0); //Disable Charging
        charge_duration_counter.enable = false;         //Stop charge timer
        state = IDLE;
    }
    
    
}

void chargingWait(void){
    Set_LED_RGB(0b111, 1023); //White LED
    if (!charge_wait_counter.enable){   //if counter isn't enabled, clear and enable it.
        charge_wait_counter.value = 0;
        charge_wait_counter.enable = true;  //Clear and start charge wait counter
    }
    else if (charge_wait_counter.value >= CHARGE_WAIT_TIMEOUT){         //2188 * 32ms = 70.016 seconds
        charge_wait_counter.enable = false;
        state = CHARGING;
    }
    
    if (detect == NONE){                    //Charger removed
        charge_wait_counter.enable = false;
        state = IDLE;
    }
    
    if (!safetyChecks()){  //Somehow there was an error
        charge_wait_counter.enable = false;
        state = ERROR;
    }
    
}

void cellBalance(void){
    
}

void outputEN(void){
        if (!ISL_GetSpecificBits_cached(ISL.ENABLE_DISCHARGE_FET)  //If discharge isn't already enabled
            && detect == TRIGGER                       //Trigger is pulled
            && ISL_GetSpecificBits_cached(ISL.WKUP_STATUS) //Make sure WKUP = 1 meaning charger connected or trigger pressed   
            && minCellOK()          //Min cell is not below low voltage cut out of 3V
            && safetyChecks()
                ){
                Set_LED_RGB(0b001, 1023); //Blue LED
                ISL_SetSpecificBits(ISL.ENABLE_DISCHARGE_FET, 1);
        }
        else if (ISL_GetSpecificBits_cached(ISL.ENABLE_DISCHARGE_FET)  //Same as above but we are already discharging and all conditions are good
            && detect == TRIGGER
            && ISL_GetSpecificBits_cached(ISL.WKUP_STATUS)
            && minCellOK()
            && safetyChecks()
                ){
                //do nothing
        }
        else if (!minCellOK()){                                 //If we hit the min cell voltage cut off, prevent discharging battery further until it is put on charger
            full_discharge_flag = true;
            ISL_SetSpecificBits(ISL.ENABLE_DISCHARGE_FET, 0);   //Disable discharging
            state = IDLE;
        }     
        else if (!safetyChecks()){
                ISL_SetSpecificBits(ISL.ENABLE_DISCHARGE_FET, 0);   //Disable discharging
                state = ERROR;
        }
        else if (detect == CHARGER){    //Charger attached while trigger was pulled
            ISL_SetSpecificBits(ISL.ENABLE_DISCHARGE_FET, 0);   //Disable discharging
            LED_code_cycle_counter.enable = true;
            uint8_t num_blinks = FIRMWARE_VERSION;
            ledBlinkpattern (num_blinks, 0b111, 500, 1000, 1000);
            if (LED_code_cycle_counter.value > 1){       //One LED cycle completed
                resetLEDBlinkPattern();
                state = IDLE;
            }
        }
        else {                                                  //Trigger released; WKUP status = 1
            ISL_SetSpecificBits(ISL.ENABLE_DISCHARGE_FET, 0);   //Disable discharging
            state = IDLE;
        }
}

void error(void){
    //TODO: Add a write to EEPROM with error info for logging
    ISL_Write_Register(FETControl, 0b00000000);     //Make sure all FETs are disabled
    
    current_error_reason = (error_reason_t){0};
    setErrorReasonFlags(&current_error_reason);
    
    static bool full_discharge_trigger_error = false;
    
    if (detect == TRIGGER && full_discharge_flag){
        full_discharge_trigger_error = true;
    }
    
    if (!current_error_reason.ISL_INT_OVERTEMP_FLAG
        && !current_error_reason.ISL_EXT_OVERTEMP_FLAG 
        && !current_error_reason.ISL_INT_OVERTEMP_PICREAD 
        && !current_error_reason.THERMISTOR_OVERTEMP_PICREAD 
        && !current_error_reason.CHARGE_OC_FLAG 
        && !current_error_reason.DISCHARGE_OC_FLAG 
        && !current_error_reason.DISCHARGE_SC_FLAG 
        && !current_error_reason.DISCHARGE_OC_SHUNT_PICREAD 
        && !current_error_reason.CHARGE_ISL_INT_OVERTEMP_PICREAD 
        && !current_error_reason.CHARGE_THERMISTOR_OVERTEMP_PICREAD 
        && !current_error_reason.TEMP_HYSTERESIS 
        && detect == NONE
        && discharge_current_mA == 0
            ){
            if (!LED_code_cycle_counter.enable){
                LED_code_cycle_counter.value = 0;
                LED_code_cycle_counter.enable = true;
            }

            /* This is error wait timeout is necessary because the while the ISL94208 datasheet claims:
             * "If the over-temperature condition has cleared, this bit is reset when the register is read."
             * regarding the external over temperature (XOT) bit, this does not appear to be true.
             * If we are continuously reading the status register, the XOT bit will only be read as asserted roughly once every 560ms.
             * It acts as if reading it is clearing the bit each time, and then it is re-asserted the next time the ISL94208
             * does it's automatic temperature scan. Oddly, occasionally we can read the XOT bit as asserted twice in a row.
             * Most of the time, even under a continuous low voltage (high temperature) on the temp. input,
             * the XOT bit will only read as asserted roughly 1 in 49 reads. */
            if (!error_timeout_wait_counter.enable){
                error_timeout_wait_counter.value = 0;
                error_timeout_wait_counter.enable = true;
            }
            else if (error_timeout_wait_counter.enable
                    && error_timeout_wait_counter.value > ERROR_EXIT_TIMEOUT
                    && LED_code_cycle_counter.enable
                    && LED_code_cycle_counter.value > NUM_OF_LED_CODES_AFTER_FAULT_CLEAR
                    ){       //three seconds must pass with no errors before error state can be exited. Also, LED code must be presented the configured number of times after fault/detect clear
                error_timeout_wait_counter.enable = false;
                sleep_timeout_counter.enable = false;
                past_error_reason = (error_reason_t){0};    //Clear error reason value for future usage
                current_error_reason = (error_reason_t){0};
                resetLEDBlinkPattern();
                full_discharge_trigger_error = false; //Reset for next usage
                state = IDLE;
                //__delay_ms(500); //This is a terrible hack to have a blank time on the LED before going back to idle
                return;
            }
        }
    else {
        error_timeout_wait_counter.enable = false;      //If there are any errors, stop the error exit timeout counter. The next time through the loop there are no errors, the counter will be reset to zero and restarted.
        LED_code_cycle_counter.enable = false;
    }

    if (past_error_reason.ISL_INT_OVERTEMP_FLAG) ledBlinkpattern (4, 0b100, 500, 1000, 1000);
    else if (past_error_reason.ISL_EXT_OVERTEMP_FLAG) ledBlinkpattern (5, 0b100, 500, 1000, 1000);
    else if (past_error_reason.ISL_INT_OVERTEMP_PICREAD) ledBlinkpattern (6, 0b100, 500, 1000, 1000);
    else if (past_error_reason.THERMISTOR_OVERTEMP_PICREAD) ledBlinkpattern (7, 0b100, 500, 1000, 1000);
    else if (past_error_reason.CHARGE_OC_FLAG) ledBlinkpattern (8, 0b100, 500, 1000, 1000);
    else if (past_error_reason.DISCHARGE_OC_FLAG) ledBlinkpattern (9, 0b100, 500, 1000, 1000);
    else if (past_error_reason.DISCHARGE_SC_FLAG) ledBlinkpattern (10, 0b100, 500, 1000, 1000);
    else if (past_error_reason.DISCHARGE_OC_SHUNT_PICREAD) ledBlinkpattern (11, 0b100, 500, 1000, 1000);
    else if (past_error_reason.CHARGE_ISL_INT_OVERTEMP_PICREAD) ledBlinkpattern (12, 0b100, 500, 1000, 1000);
    else if (past_error_reason.CHARGE_THERMISTOR_OVERTEMP_PICREAD) ledBlinkpattern (13, 0b100, 500, 1000, 1000);
    else if (full_discharge_trigger_error) ledBlinkpattern (3, 0b001, 500, 0, 0);       //trigger is pulled but battery is low
    else ledBlinkpattern (20, 0b100, 500, 1000, 1000);                                                                  //Unidentified Error
    
    
    
    
    
    if (sleep_timeout_counter.enable == false   //If there is an error, start sleep counter (if it isn't already started), so we sleep if in error state for too long
            && detect == NONE){                 //Also, don't start sleep sequence if we are connected to charger or someone is holding trigger
        sleep_timeout_counter.value = 0;
        sleep_timeout_counter.enable = true;
    }
    else if (detect != NONE){                   //If at any point the charger is connected or trigger pressed, abort sleep sequence.
        sleep_timeout_counter.enable = false;
    }
    else if (sleep_timeout_counter.value >  ERROR_SLEEP_TIMEOUT //1876*32ms = 60.032s //If we are in ERROR state for 60 seconds, just go to sleep.
            && sleep_timeout_counter.enable == true
            && nonblocking_wait_counter.enable == false     //Don't sleep in the middle of an LED blink code cycle
            && nonblocking_wait_counter.value == 0
            && detect == NONE
            ){    
        sleep_timeout_counter.enable = false;
        state = SLEEP;
        //TODO: Make sure it will actually sleep if trigger is still held down so WKUP STATUS == 1
    }
    
}
#ifdef __DEBUG
    volatile uint16_t loop_counter = 0;
#endif
    
void main(void)
{
    
    
    init();
    
    while (1)
    {
        #ifdef __DEBUG
        loop_counter++;
        #endif  
        
        if (!ISL_GetSpecificBits(ISL.WKPOL)){//If somehow the ISL was reset and so WKPOL isn't correct, reinitialize everything and clear the I2C bus.
            __debug_break();
            I2C1_Init();
            ClearI2CBus();
            I2C_ERROR_FLAGS = 0;    //Clear error flags
        }
        
        ISL_ReadAllCellVoltages();
        ISL_calcCellStats();
        detect = checkDetect();
//        if (state == IDLE && detect == TRIGGER){
//            __debug_break();
//        }
        isl_int_temp = ISL_GetInternalTemp();
        thermistor_temp = getThermistorTemp(modelnum);
        
        //For testing - Allows reading of actual values from these sensors when defined in config.h.
        //Overwrites the real value with 25 which will always be an acceptable temperature so that limit is effectively disabled.
        #ifdef __DEBUG_DISABLE_PIC_THERMISTOR_READ
        thermistor_temp = 25;
        #endif

        #ifdef __DEBUG_DISABLE_PIC_ISL_INT_READ
        isl_int_temp = 25;
        #endif
        
        ISL_Read_Register(Config);      //Get config register so we can check WKUP status later on
        ISL_Read_Register(Status);      //Get Status register to check for error flags
        ISL_Read_Register(FETControl);  //Get current FET status
        discharge_current_mA = dischargeIsense_mA();
        
            //TODO: Add I2C error check here
        
        
        switch(state){
            case INIT:
                init();
                break;
                
            case SLEEP:
                sleep();
                break;
            
            case IDLE:
                idle();
                break;
                
            case CHARGING:
                charging();
                break;
                
            case CHARGING_WAIT:
                chargingWait();
                break;

            case CELL_BALANCE:
                cellBalance();
                break;
                
            case OUTPUT_EN:
                outputEN();
                break;

            case ERROR:
                error();
                break;
                
        }
        
        if (TMR4_HasOverflowOccured()){         //Every 32ms 
            if (charge_wait_counter.enable){
                charge_wait_counter.value++;
            }
            
            if (charge_duration_counter.enable){
                charge_duration_counter.value++;
            }
            
            if (sleep_timeout_counter.enable){
                sleep_timeout_counter.value++;
            }
            
            if (nonblocking_wait_counter.enable){
                nonblocking_wait_counter.value++;
            }
            
            if (error_timeout_wait_counter.enable){
                error_timeout_wait_counter.value++;
            }
            
        
        
        }


        
//        #ifdef __DEBUG
//        for (uint8_t i = 0; i < __ISL_NUMBER_OF_REG; i++){
//            ISL_Read_Register(i);
//        }
//        #endif
        




        
        
    }
}

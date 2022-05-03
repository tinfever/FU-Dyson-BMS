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

#include "mcc_generated_files/mcc.h"
#include "main.h"
#include "i2c.h"
#include "isl94208.h"
#include "config.h"
#include "thermistor.h"
#include "LED.h"
#include "FaultHandling.h"

//EEPROM Init during programming
__EEPROM_DATA(0x54, 0x69, 0x6E, 0x66, 0x65, 0x76, 0x65, 0x72);              //"Tinfever"    EEPROM addresses 0x00 - 0x07
__EEPROM_DATA(0x20, 0x46, 0x55, 0x2D, 0x44, 0x79, 0x73, 0x6F);              //" FU-Dyso"    EEPROM addresses 0x08 - 0x0F
__EEPROM_DATA(0x6E, 0x2D, 0x42, 0x4D, 0x53, 0x20, 0x56, ASCII_FIRMWARE_VERSION);  //"n-BMS V{insert firmware version here}"     EEPROM addresses 0x10 - 0x17
__EEPROM_DATA(0, EEPROM_START_OF_EVENT_LOGS_ADDR, 0, 0, 0, 0, 0, 0);                                     //Address of the next available space for recording error events       EEPROM addresses 0x17 - 0x1F
//

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
        if (SDA == 1 && SCL == 1){  //Read data and check if SDA is high (idle). Also make sure SCL isn't getting glitched low
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



////////////////////////////////////////////////////////////////////



void init(void){
//INIT STEPS
    
    I2C_ERROR_FLAGS = 0;
    
    /* Initialize the device */
    SYSTEM_Initialize();
    TMR4_StartTimer();   //Keep timer running
    DAC_SetOutput(0);   //Make sure DAC output is 0V = VSS

    //* Set up I2C pins */
    TRIS_SDA = 1;     //SDA - Make sure both pins are inputs
    TRIS_SCL = 1;     //SCL
    ANS_SDA = 0;    //Set both pins as digital
    ANS_SCL = 0;
    I2C1_Init();
    ClearI2CBus();  //Clear I2C bus once on startup just in case
    while (SDA == 0 || SCL == 0){   //If bus is still not idle (meaning pins aren't high), which shouldn't be possible, then keep trying to clear bus. Do not pass go. Do not collect $200.
    //    __debug_break();
        ClearI2CBus();
    }
    
    
    modelnum = checkModelNum();    
    
    //Load 32-bit total runtime counter from EEPROM
    total_runtime_counter.value = (uint32_t) DATAEE_ReadByte(EEPROM_RUNTIME_TOTAL_STARTING_ADDR) << 24;
    total_runtime_counter.value |= (uint32_t) DATAEE_ReadByte(EEPROM_RUNTIME_TOTAL_STARTING_ADDR+1) << 16;
    total_runtime_counter.value |= (uint32_t) DATAEE_ReadByte(EEPROM_RUNTIME_TOTAL_STARTING_ADDR+2) << 8;
    total_runtime_counter.value |= (uint32_t) DATAEE_ReadByte(EEPROM_RUNTIME_TOTAL_STARTING_ADDR+3);
    
    
    //INIT END
    
    state = IDLE;
}

void sleep(void){
#ifdef __DEBUG_DONT_SLEEP
    state = IDLE;
    return;
#endif
    
    ISL_SetSpecificBits(ISL.SLEEP, 1);
    __delay_us(50);
    ISL_SetSpecificBits(ISL.SLEEP, 0);
    __delay_us(50);
    ISL_SetSpecificBits(ISL.SLEEP, 1);
     __delay_ms(250);
     ClearI2CBus(); //If the ISL didn't actually sleep when we just told it to, something is seriously wrong. The best we can do is to try to reset the ISL.
     ISL_Init();    //This includes a POR reset of the ISL

}

void idle(void){
    static bool previous_detect_was_charger = 0;
    static bool show_cell_delta_LEDs = 1;
    
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
        if ((show_cell_delta_LEDs == 1 && cellDeltaLEDIndicator())
            || !show_cell_delta_LEDs
            ) {
            state = CHARGING;
        }
        
        
    }
    else if ((detect == NONE                         //Start sleep counter if we are idle with no charger or trigger, but no errors
#ifdef SLEEP_AFTER_CHARGE_COMPLETE
            || (detect == CHARGER && charge_complete_flag) //Also sleep after charge is complete while we are on the charger, if configured.
#endif
            )
#ifndef SLEEP_AFTER_CHARGE_COMPLETE
            && ISL_GetSpecificBits_cached(ISL.WKUP_STATUS) == 0
#endif
            && sleep_timeout_counter.enable == false
            && safetyChecks()
            ){
                sleep_timeout_counter.value = 0;        //Clear and start sleep counter
                sleep_timeout_counter.enable = true;
                show_cell_delta_LEDs = 1;   //Reset this just in case
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
    else if (detect == NONE){                               
        if (CheckStateInDetectHistory(CHARGER)){                                //Show cell delta code when charger removed after complete charge
            previous_detect_was_charger = true;                             //This flag is set if we are transitioning from detect == CHARGER to detect == NONE
        }
        
        if ((previous_detect_was_charger && cellDeltaLEDIndicator())
            || !previous_detect_was_charger    
        ){       //If the Charger -> None transition was detected, keep checking/running the cellDeltaLEDIndicator function until it is complete. 
            previous_detect_was_charger = false;                            //Then remove flag
            Set_LED_RGB(0b010, 1023); //LED Green
            show_cell_delta_LEDs = 1; //Set this just in case
        }
    }
    else if (detect == TRIGGER && ISL_GetSpecificBits_cached(ISL.WKUP_STATUS) && !full_discharge_flag){    //trigger is pulled but we didn't enable output, and it isn't because there was an error or the pack is fully discharged = Yellow LED
            //There is a delay inside ISL IC between when detect == TRIGGER and when WKUP == 1. adding && WKUP_STATUS make sure both are in the same state to avoid brief yellow LED flash.
        Set_LED_RGB(0b110, 1023); //LED Yellow    
    }
    
    //There is no handling for WKUP_STATUS and DETECT to be in different states. If WKUP_STATUS == 0 (don't wakeup) but DETECT == TRIGGER, God help us all.
    
    //If the charger is connected, we are in a fully charged idle state, the user holds down the trigger, and then disconnects the charger while holding the trigger, the cell balance LED indicator is not shown. Not worth it to implement.
    
    if (charge_complete_flag == true && cellstats.maxcell_mV < PACK_CHARGE_NOT_COMPLETE_THRESH_mV){      //If the max cell voltage is below 4100mV and the pack is marked as fully charged, unmark it as charged.
        charge_complete_flag = false;
        show_cell_delta_LEDs = 0;
    }
    
    if (detect != CHARGER){                  //Also, if removed from charger, clear charge complete flag.
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
        previous_detect_was_charger = false;
        show_cell_delta_LEDs = 1;
    }
    
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
            resetLEDBlinkPattern();
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
    else{                                   //charger removed before complete charge
        ISL_SetSpecificBits(ISL.ENABLE_CHARGE_FET, 0); //Disable Charging
        charge_duration_counter.enable = false;         //Stop charge timer
        state = IDLE;
        
        
    }
    //Clean up before state change
    if (state != CHARGING){
        resetLEDBlinkPattern();
    }
    
}

void chargingWait(void){
    if (detect == CHARGER){                     //Don't set the LED like this if the charger isn't connected because it will interfere with the cellDeltaLEDIndicator
        Set_LED_RGB(0b111, 1023); //White LED
    }
    
    if (!charge_wait_counter.enable){   //if counter isn't enabled, clear and enable it.
        charge_wait_counter.value = 0;
        charge_wait_counter.enable = true;  //Clear and start charge wait counter
    }
    else if (charge_wait_counter.value >= CHARGE_WAIT_TIMEOUT){         //2188 * 32ms = 70.016 seconds
        charge_wait_counter.enable = false;
        state = CHARGING;
    }
    
    if (detect != CHARGER){                    //Charger removed
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
    static uint8_t startup_led_step = 0;
    static bool runonce = 0;
    static bool need_to_clear_LEDs_for_cell_voltage_indicator = 1;

        if (!ISL_GetSpecificBits_cached(ISL.ENABLE_DISCHARGE_FET)  //If discharge isn't already enabled
            && detect == TRIGGER                       //Trigger is pulled
            && ISL_GetSpecificBits_cached(ISL.WKUP_STATUS) //Make sure WKUP = 1 meaning charger connected or trigger pressed   
            && minCellOK()          //Min cell is not below low voltage cut out of 3V
            && safetyChecks()
                ){
                ISL_SetSpecificBits(ISL.ENABLE_DISCHARGE_FET, 1);
                startup_led_step = 0;
                resetLEDBlinkPattern();
                need_to_clear_LEDs_for_cell_voltage_indicator = 1;  //Make sure the LED blink pattern state is reset when trigger is released so we get clean blink pattern
                runonce = 0;
                total_runtime_counter.enable = true;
                LED_code_cycle_counter.value = 0;
                
        }
        else if (ISL_GetSpecificBits_cached(ISL.ENABLE_DISCHARGE_FET)  //Same as above but we are already discharging and all conditions are good
            && detect == TRIGGER
            && ISL_GetSpecificBits_cached(ISL.WKUP_STATUS)
            && minCellOK()
            && safetyChecks()
                ){
                //Fancy start up LEDs
                need_to_clear_LEDs_for_cell_voltage_indicator = 1;
                runonce = 0;
                switch(startup_led_step){
                    case 0:
                        LED_code_cycle_counter.enable = true;
                        ledBlinkpattern (1, 0b100, 1000, 0, 0, 0, 32);
                        if (LED_code_cycle_counter.value > 1){
                            startup_led_step++;
                            resetLEDBlinkPattern();
                        }
                        break;

                    case 1:
                        LED_code_cycle_counter.enable = true;
                        ledBlinkpattern (1, 0b010, 1000, 0, 0, 0, 32);
                        if (LED_code_cycle_counter.value > 1){
                            startup_led_step++;
                            resetLEDBlinkPattern();
                        }
                        break;

                    case 2: 
                        LED_code_cycle_counter.enable = true;
                        ledBlinkpattern (1, 0b001, 1000, 0, 0, 0, 32);
                        if (LED_code_cycle_counter.value > 1){
                            startup_led_step++;
                            nonblocking_wait_counter.enable = false;        //Same as resetLEDBlinkPattern but without turning off the LED
                            nonblocking_wait_counter.value = 0;
                            LED_code_cycle_counter.enable = false;
                            LED_code_cycle_counter.value = 0;
                        }
                        break;

                        default:
                            Set_LED_RGB(0b001, 1023);       //Set LED to blue after fancy startup LEDs
                            break;
            }
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
            need_to_clear_LEDs_for_cell_voltage_indicator = 1;
            
            if (!runonce){
                resetLEDBlinkPattern();
                LED_code_cycle_counter.enable = true;
                runonce = true;
            } 
            
            uint8_t num_blinks = FIRMWARE_VERSION;
            ledBlinkpattern (num_blinks, 0b111, 500, 500, 1000, 1000, 0);
            if (LED_code_cycle_counter.value > 1){       //One LED cycle completed
                state = IDLE;
            }
        }
        else {                                                  //Trigger released; WKUP status = 1
            ISL_SetSpecificBits(ISL.ENABLE_DISCHARGE_FET, 0);   //Disable discharging
            runonce = 0;
            if (need_to_clear_LEDs_for_cell_voltage_indicator == 1){    //Prevents weird things from happening if you release the trigger in the middle of the startup LED sequence
                resetLEDBlinkPattern();
                need_to_clear_LEDs_for_cell_voltage_indicator = 0;
            }
            if (cellVoltageLEDIndicator()){   //Repeat the cellVoltageLEDIndicator() function until one iteration, when it will return true.
                state = IDLE;
            }
        }
    
    //State change cleanup
    if (state != OUTPUT_EN){
        total_runtime_counter.enable = false;
        
        //Write latest runtime counter value to EEPROM
        WriteTotalRuntimeCounterToEEPROM(EEPROM_RUNTIME_TOTAL_STARTING_ADDR);
        
        
        startup_led_step = 0;
        runonce = 0;
        need_to_clear_LEDs_for_cell_voltage_indicator = 1;
        resetLEDBlinkPattern();
    }
    
}

void error(void){
    ISL_Write_Register(FETControl, 0b00000000);     //Make sure all FETs are disabled
    
    static bool EEPROM_Event_Logged = false;

    current_error_reason = (error_reason_t){0};
    setErrorReasonFlags(&current_error_reason);
    
    static bool full_discharge_trigger_error = false;
    if (detect == TRIGGER && full_discharge_flag){
        full_discharge_trigger_error = true;
    }
    
    static bool critical_i2c_error = false;
    if (!(I2C_error_counter < CRITICAL_I2C_ERROR_THRESH)){
        critical_i2c_error = true;
    }
    
    if (!EEPROM_Event_Logged && !full_discharge_trigger_error){    //full_discharge_trigger_error isn't an actual error that needs recording
        const uint8_t byte_size_of_event_log = 6;
        uint8_t starting_write_addr = DATAEE_ReadByte(EEPROM_NEXT_BYTE_AVAIL_STORAGE_ADDR);
        
        //Assemble first byte
        uint8_t data_byte_1 = 0;
        data_byte_1 |= past_error_reason.ISL_INT_OVERTEMP_FLAG << 7;                //Due to bit-field used in declaration, these can only be one bit.
        data_byte_1 |= past_error_reason.ISL_EXT_OVERTEMP_FLAG << 6;
        data_byte_1 |= past_error_reason.ISL_INT_OVERTEMP_PICREAD << 5;
        data_byte_1 |= past_error_reason.THERMISTOR_OVERTEMP_PICREAD << 4;
        data_byte_1 |= past_error_reason.UNDERTEMP_FLAG << 3;
        data_byte_1 |= past_error_reason.CHARGE_OC_FLAG << 2;
        data_byte_1 |= past_error_reason.DISCHARGE_OC_FLAG << 1;
        data_byte_1 |= past_error_reason.DISCHARGE_SC_FLAG;
        
        //Assemble second byte
        uint8_t data_byte_2 = 0;
        data_byte_2 |= past_error_reason.DISCHARGE_OC_SHUNT_PICREAD << 7;
        data_byte_2 |= past_error_reason.CHARGE_ISL_INT_OVERTEMP_PICREAD << 6;
        data_byte_2 |= past_error_reason.CHARGE_THERMISTOR_OVERTEMP_PICREAD << 5;
        data_byte_2 |= past_error_reason.TEMP_HYSTERESIS << 4;                          //This point might be useless
        data_byte_2 |= past_error_reason.ISL_BROWN_OUT << 3;
        data_byte_2 |= critical_i2c_error  << 2;
        data_byte_2 |= (past_error_reason.DETECT_MODE & 0b00000011);
        
        //Write data to EEPROM
        DATAEE_WriteByte(starting_write_addr, data_byte_1);
        DATAEE_WriteByte(starting_write_addr+1, data_byte_2);
        WriteTotalRuntimeCounterToEEPROM(starting_write_addr+2);
        
        uint8_t future_starting_write_addr = EEPROM_START_OF_EVENT_LOGS_ADDR;           //Default to the starting address of 0x20
        if (     !( ((uint16_t) starting_write_addr + byte_size_of_event_log + byte_size_of_event_log - 1) > 255 )     ){        //Make sure that there is enough room left after we write our 6 bytes. Subtract one because we are checking if the ending of the next event would be out of bounds.
            future_starting_write_addr = starting_write_addr + byte_size_of_event_log;
        }
        
        DATAEE_WriteByte(EEPROM_NEXT_BYTE_AVAIL_STORAGE_ADDR, future_starting_write_addr);   //Record next available free space for future event records
        EEPROM_Event_Logged = true;
    }
    
        /* This is a dirty hack to handle a possible hardware bug where:
         * 1) The trigger is pulled and output is enabled as usual
         * 2) The short circuit protection kicks in due to an apparent short
         * 3) The output is disabled as expected
         * 4) The ISL94208 RESETS ITSELF AND DOES NOT PROVIDE THE SHORT CIRCUIT ERROR FLAG. Thus it is not obvious what happened.
         * 5) The routine I2C commands to the ISL fail while it is resetting, causing I2C errors.
         * 6) Previously, I2C errors were handled by resetting the PIC. So the PIC is reset.
         * 7) The PIC starts up and sees the trigger is pulled and the ISL is presenting no error flags.
         * 8) Wash, rinse, repeat. 
         * 
         * Now, we are setting user flag bits 0 and 1 during setup and routinely checking to make sure those are still set.
         * If those are ever cleared, that means the ISL reset itself. This causes the ISL_BROWN_OUT error code.
         * 
         * This could probably be integrated in to the normal fault handling system much more cleanly,
         * and without having to create an infinite loop and duplicate some of the main loop routines.
         * However, at this point I can't be bothered and so I've just hacked this in.
         * This is the simplest way of handling a situation where I'm not sure I can trust anything is initialized the way it should be.
         *  
        */
    if (critical_i2c_error || past_error_reason.ISL_BROWN_OUT){         
        LED_code_cycle_counter.value = 0;
        while(1){                                               //It's called critical for a reason
            if (!detect){
                LED_code_cycle_counter.enable = true;       //Start led error code sequence after trigger released and/or charger disconnected
            } 
            else{
                LED_code_cycle_counter.value = 0;           //Attaching charger or pulling trigger will reset LED code count
                LED_code_cycle_counter.enable = false;
            }

            if (LED_code_cycle_counter.value > NUM_OF_LED_CODES_AFTER_FAULT_CLEAR){
                RESET();        //Once required number of error codes are shown, use the nuclear option.
            }
            
            
            if (past_error_reason.ISL_BROWN_OUT){
                ledBlinkpattern(16, 0b100, 500, 500, 1000, 1000, 0);    //ISL brown out
            }
            else{
                ledBlinkpattern(15, 0b100, 500, 500, 1000, 1000, 0);    //critical i2c error
            }
            
            if (TMR4_HasOverflowOccured()){         //Every 32ms //Since we aren't going in to main loop again, we still have to service this counter for the LED code to work
                if (nonblocking_wait_counter.enable){
                    nonblocking_wait_counter.value++;
                }
            }
            
            CLRWDT();   //We also have to clear the WDT
            
            detect = checkDetect();     //And check the latest detect value
            
        }
    }
    
    if (!current_error_reason.ISL_INT_OVERTEMP_FLAG
        && !current_error_reason.ISL_EXT_OVERTEMP_FLAG 
        && !current_error_reason.ISL_INT_OVERTEMP_PICREAD 
        && !current_error_reason.THERMISTOR_OVERTEMP_PICREAD 
        && !current_error_reason.UNDERTEMP_FLAG
        && !current_error_reason.CHARGE_OC_FLAG 
        && !current_error_reason.DISCHARGE_OC_FLAG 
        && !current_error_reason.DISCHARGE_SC_FLAG 
        && !current_error_reason.DISCHARGE_OC_SHUNT_PICREAD 
        && !current_error_reason.CHARGE_ISL_INT_OVERTEMP_PICREAD 
        && !current_error_reason.CHARGE_THERMISTOR_OVERTEMP_PICREAD 
        && !current_error_reason.TEMP_HYSTERESIS 
        && ((detect == NONE) || (full_discharge_trigger_error && detect == CHARGER))    //if the error reason was being fully discharged, allow exit loop if device is connected to charger
        && discharge_current_mA == 0
            ){
            if (!LED_code_cycle_counter.enable){
                LED_code_cycle_counter.value = 0;
                LED_code_cycle_counter.enable = true;
            }

            /* Error wait timeout is necessary because the while the ISL94208 datasheet claims:
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
                EEPROM_Event_Logged = false;
                state = IDLE;
                return;
            }
        }
    else {
        error_timeout_wait_counter.enable = false;      //If there are any errors, stop the error exit timeout counter. The next time through the loop there are no errors, the counter will be reset to zero and restarted.
        LED_code_cycle_counter.enable = false;
    }

    if (past_error_reason.ISL_INT_OVERTEMP_FLAG) ledBlinkpattern (4, 0b100, 500, 500, 1000, 1000, 0);
    else if (past_error_reason.ISL_EXT_OVERTEMP_FLAG) ledBlinkpattern (5, 0b100, 500, 500, 1000, 1000, 0);
    else if (past_error_reason.ISL_INT_OVERTEMP_PICREAD) ledBlinkpattern (6, 0b100, 500, 500, 1000, 1000, 0);
    else if (past_error_reason.THERMISTOR_OVERTEMP_PICREAD) ledBlinkpattern (7, 0b100, 500, 500, 1000, 1000, 0);
    else if (past_error_reason.CHARGE_OC_FLAG) ledBlinkpattern (8, 0b100, 500, 500, 1000, 1000, 0);
    else if (past_error_reason.DISCHARGE_OC_FLAG) ledBlinkpattern (9, 0b100, 500, 500, 1000, 1000, 0);
    else if (past_error_reason.DISCHARGE_SC_FLAG) ledBlinkpattern (10, 0b100, 500, 500, 1000, 1000, 0);
    else if (past_error_reason.DISCHARGE_OC_SHUNT_PICREAD) ledBlinkpattern (11, 0b100, 500, 500, 1000, 1000, 0);
    else if (past_error_reason.CHARGE_ISL_INT_OVERTEMP_PICREAD) ledBlinkpattern (12, 0b100, 500, 500, 1000, 1000, 0);
    else if (past_error_reason.CHARGE_THERMISTOR_OVERTEMP_PICREAD) ledBlinkpattern (13, 0b100, 500, 500, 1000, 1000, 0);
    else if (past_error_reason.UNDERTEMP_FLAG) ledBlinkpattern (14, 0b100, 500, 500, 1000, 1000, 0);
    else if (full_discharge_trigger_error) ledBlinkpattern (3, 0b001, 300, 300, 750, 750, 0);       //trigger is pulled but battery is low
    else ledBlinkpattern (20, 0b100, 500, 500, 1000, 1000, 0);                                                                  //Unidentified Error
    
    
    
    
    
    if (sleep_timeout_counter.enable == false   //If there is an error, start sleep counter (if it isn't already started), so we sleep if in error state for too long
            && detect != CHARGER){                 //Also, don't start sleep sequence if we are connected to charger
        sleep_timeout_counter.value = 0;
        sleep_timeout_counter.enable = true;
    }
    else if (detect == CHARGER){                   //If at any point the charger is connected, abort sleep sequence. I'm not quite sure why we'd want to though.
        sleep_timeout_counter.enable = false;
    }
    else if (sleep_timeout_counter.value >  ERROR_SLEEP_TIMEOUT //1876*32ms = 60.032s //If we are in ERROR state for 60 seconds, just go to sleep.
            && sleep_timeout_counter.enable == true
            && nonblocking_wait_counter.enable == false     //Don't sleep in the middle of an LED blink code cycle
            && nonblocking_wait_counter.value == 0
            && detect != CHARGER
            ){    
        sleep_timeout_counter.enable = false;
        state = SLEEP;
        
    }
    
}
#ifdef __DEBUG
    volatile uint16_t loop_counter = 0;
#endif
        
void RecordDetectHistory(void){
    detect_history = (uint8_t) ( (uint8_t) (detect_history << 2) | (detect & 0b00000011) );
}
    
detect_t GetDetectHistory(uint8_t position){
    return (detect_t) ((detect_history >> (2*position)) & 0b00000011);
}

bool CheckStateInDetectHistory(detect_t detect_val){
    for (uint8_t i = 0; i < 4; i++){
        if(GetDetectHistory(i) == detect_val){
            return true;
        }
    }
    return false;
}

void WriteTotalRuntimeCounterToEEPROM(uint8_t starting_addr){       //Make sure there are four bytes available with the provided starting address
    DATAEE_WriteByte(starting_addr, (uint8_t) ((total_runtime_counter.value & 0xFF000000) >> 24)     );
    DATAEE_WriteByte(starting_addr+1, (uint8_t) ((total_runtime_counter.value & 0xFF0000) >> 16) );
    DATAEE_WriteByte(starting_addr+2, (uint8_t) ((total_runtime_counter.value & 0xFF00) >> 8)    );
    DATAEE_WriteByte(starting_addr+3, (uint8_t) (total_runtime_counter.value & 0xFF)   );
}

void main(void)
{    
    init();
    
    while (1)
    {
        CLRWDT();
        //__delay_ms(5);
        #ifdef __DEBUG
        loop_counter++;
        #endif  

        ISL_Read_Register(AnalogOut); //Get Analog Out register that contains user bits
        ISL_BrownOutHandler();
        
        ISL_ReadAllCellVoltages();
        ISL_calcCellStats();
        RecordDetectHistory();
        detect = checkDetect();
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
        ISL_Read_Register(AnalogOut); //Get Analog Out register that contains user bits
        discharge_current_mA = dischargeIsense_mA();
        
        if (ISL_BrownOutHandler()){
            //do nothing
        }
        else if (I2C_ERROR_FLAGS != 0){     //I2C error handling
            I2C_error_counter++;

            if (I2C_error_counter < CRITICAL_I2C_ERROR_THRESH) {
                I2C1_Init();
                ClearI2CBus(); //Clear error flags  //First try just clearing I2C bus (which will also POR reset ISL94208)  
                continue; //Then go again from the top.
            } else {
                state = ERROR;
            }
        } else {        //If we were successful this time, clear the error counter.
            I2C_error_counter = 0;
        }
        
        
        
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
            
            if (total_runtime_counter.enable){
                total_runtime_counter.value++;
            }
            
            
        
        
        }
        
        
    }
}

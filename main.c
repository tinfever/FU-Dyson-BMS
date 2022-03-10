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

void Set_LED_RGB(uint8_t RGB){  //Accepts binary input 0b000. Bit 2 = Red Enable. Bit 1 = Green Enable. Bit 0 = Red Enable. R.G.B.
    if (RGB & 0b001){
        blueLED = 0;
    }
    else{
        blueLED = 1;
    }
    
    if (RGB & 0b010){
        greenLED = 0;
    }
    else{
        greenLED = 1;
    }
    
    if (RGB & 0b100){
        redLED = 0;
    }
    else{
        redLED = 1;
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

void init(void){
//INIT STEPS
    
    I2C_ERROR_FLAGS = 0;
    
    /* Initialize the device */
    SYSTEM_Initialize();
    TMR4_StartTimer();   //Keep timer running
    DAC_SetOutput(0);   //Make sure DAC output is 0V = VSS
    
    /* Initialize LEDS */
    redLED = 1;  //Set high to default RED led to off
    greenLED = 1;  //Set high to default GREEN led to off
    blueLED = 1;  //Set high to default BLUE led to off
    TRISB3 = 0; // Set greenLED as output
    TRISA6 = 0; //Set blueLED as output
    TRISA7 = 0; //Set redLED as output
    ANSB3 = 0; //Set Green LED as digital. Red and Blue are always digital.

    /* Immediately start yellow LED so we know board is alive */
    Set_LED_RGB(0b110);

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
    modelnum = checkModelNum();
    //INIT END
    
    state = IDLE;
}

void sleep(void){
    ISL_SetSpecificBits(ISL.SLEEP, 1);
}

void idle(void){
    Set_LED_RGB(0b010); //LED Green
    //TODO: Add I2C error check here
    
    if (detect == TRIGGER                       //Trigger is pulled
        && cellstats.mincell_mV > MIN_DISCHARGE_CELL_VOLTAGE_mV          //Min cell is not below low voltage cut out of 3V
        && isl_int_temp < MAX_CHARGE_TEMP_C     //Temps are OK
        && thermistor_temp < MAX_CHARGE_TEMP_C
        && ISL_GetSpecificBits(ISL.WKUP_STATUS) //Make sure WKUP = 1 meaning charger connected or trigger pressed
        && ISL_RegData[Status] == 0){           //No errors
            sleep_timeout_counter.enable = false;   //We aren't going to be sleeping soon
            state = OUTPUT_EN;
    }
    else if (detect == CHARGER                       //Charger is connected
        && charge_complete_flag == false         //We haven't already done a complete charge cycle
        && cellstats.maxcell_mV < MAX_CHARGE_CELL_VOLTAGE_mV          //Max cell < 4.20V
        && isl_int_temp < MAX_CHARGE_TEMP_C     //Make sure temps are OK
        && thermistor_temp < MAX_CHARGE_TEMP_C
        && ISL_GetSpecificBits(ISL.WKUP_STATUS) //Make sure WKUP = 1 meaning charger connected or trigger pressed
        && ISL_RegData[Status] == 0){           //Verify no status error flags
            sleep_timeout_counter.enable = false;   //We aren't going to be sleeping soon
            state = CHARGING; 
    }
    else if (ISL_RegData[Status] != 0){  //Somehow there was an error
        sleep_timeout_counter.enable = false;
        state = ERROR;
        //TODO: add checking for other error types. Make sure to add exit methods to error state function as well.
    }
    
    if ( (charge_complete_flag == true && cellstats.maxcell_mV < 4100) || (detect != CHARGER)){      //If the max cell voltage is below 4100mV and the pack is marked as fully charged, unmark it as charged. Also, if removed from charger, clear charge complete flag.
        charge_complete_flag = false;
    }
    
    if (detect == NONE && ISL_GetSpecificBits(ISL.WKUP_STATUS) == 0 && sleep_timeout_counter.enable == false){  //Start sleep counter if nothing is connected and it isn't already running
        sleep_timeout_counter.value = 0;
        sleep_timeout_counter.enable = true;
    }
    
    if (sleep_timeout_counter.value >  IDLE_SLEEP_TIMEOUT && sleep_timeout_counter.enable == true){    //938*32ms = 30.016s //If we are in IDLE state for 30 seconds and not on the charger, go to sleep. We will stay awake on the charger since we have power to spare and can then make sure battery voltages don't drop over time.
        sleep_timeout_counter.enable = false;
        state = SLEEP;
    }
    
    
}

void charging(void){

    if (!ISL_GetSpecificBits(ISL.ENABLE_CHARGE_FET)     //if we aren't already charging
        && detect == CHARGER
        && cellstats.maxcell_mV < MAX_CHARGE_CELL_VOLTAGE_mV
        && isl_int_temp < MAX_CHARGE_TEMP_C
        && thermistor_temp < MAX_CHARGE_TEMP_C
        && ISL_GetSpecificBits(ISL.WKUP_STATUS)
        && ISL_RegData[Status] == 0)
        {
        charge_duration_counter.value = 0;
        charge_duration_counter.enable = true;          //Start charge timer
        ISL_SetSpecificBits(ISL.ENABLE_CHARGE_FET, 1);  //Enable Charging
        Set_LED_RGB(0b111); //White LED
    }
    else if (ISL_GetSpecificBits(ISL.ENABLE_CHARGE_FET)     //same as above but we are already charging and all conditions are good
        && detect == CHARGER
        && cellstats.maxcell_mV < MAX_CHARGE_CELL_VOLTAGE_mV
        && isl_int_temp < MAX_CHARGE_TEMP_C
        && thermistor_temp < MAX_CHARGE_TEMP_C
        && ISL_GetSpecificBits(ISL.WKUP_STATUS)
        && ISL_RegData[Status] == 0){
        //do nothing
    }
    else if (cellstats.maxcell_mV >= MAX_CHARGE_CELL_VOLTAGE_mV){         //Target voltage reached
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
    else if (ISL_RegData[Status] != 0){     //There was an error
        ISL_SetSpecificBits(ISL.ENABLE_CHARGE_FET, 0); //Disable Charging
        charge_duration_counter.enable = false;         //Stop charge timer
        if (ISL_RegData[Status] & 0b1){     //Error was charge overcurrent flag
            state = CHARGE_OC_LOCKOUT; 
        }
        else{   //It must be either an int over-temp, ext over-temp, discharge SC/OC error (discharge shouldn't be possible though)
            state = ERROR;
        }
        
    }
    else{                                   //charger removed or manual temp read overtemp
        ISL_SetSpecificBits(ISL.ENABLE_CHARGE_FET, 0); //Disable Charging
        charge_duration_counter.enable = false;         //Stop charge timer
        state = IDLE;
    }
    
    
}

void chargingWait(void){
    Set_LED_RGB(0b001); //Blue LED
    if (!charge_wait_counter.enable){   //if counter isn't enabled, clear and enable it.
        charge_wait_counter.value = 0;
        charge_wait_counter.enable = true;  //Clear and start charge wait counter
    }
    
    if (charge_wait_counter.value >= CHARGE_WAIT_TIMEOUT){         //2188 * 32ms = 70.016 seconds
        charge_wait_counter.enable = false;
        state = CHARGING;
    }
    
    if (detect == NONE){                    //Charger removed
        charge_wait_counter.enable = false;
        state = IDLE;
    }
    
    if (ISL_RegData[Status] != 0){  //Somehow there was an error
        charge_wait_counter.enable = false;
        state = ERROR;
    }
    
}

void chargeOClockout(void){
    Set_LED_RGB(0b100); //Red LED
    if (detect == NONE){    //Stay in charge OC lockout state until charger is disconnected
        state = IDLE;
    }
}

void cellBalance(void){
    
}

void outputEN(void){
        if (!ISL_GetSpecificBits(ISL.ENABLE_DISCHARGE_FET)  //If discharge isn't already enabled
            && detect == TRIGGER                       //Trigger is pulled
            && cellstats.mincell_mV > MIN_DISCHARGE_CELL_VOLTAGE_mV          //Min cell is not below low voltage cut out of 3V
            && isl_int_temp < MAX_CHARGE_TEMP_C     //Temps are OK
            && thermistor_temp < MAX_CHARGE_TEMP_C
            && ISL_GetSpecificBits(ISL.WKUP_STATUS) //Make sure WKUP = 1 meaning charger connected or trigger pressed
            && ISL_RegData[Status] == 0 //No errors
            && discharge_current_mA < MAX_DISCHARGE_CURRENT_mA){     //We aren't discharging more than 30A
                Set_LED_RGB(0b111); //White LED
                ISL_SetSpecificBits(ISL.ENABLE_DISCHARGE_FET, 1);
        }
        else if (ISL_GetSpecificBits(ISL.ENABLE_DISCHARGE_FET)  //Same as above but we are already discharging and all conditions are good
            && detect == TRIGGER
            && cellstats.mincell_mV > MIN_DISCHARGE_CELL_VOLTAGE_mV 
            && isl_int_temp < MAX_CHARGE_TEMP_C
            && thermistor_temp < MAX_CHARGE_TEMP_C
            && ISL_GetSpecificBits(ISL.WKUP_STATUS)
            && ISL_RegData[Status] == 0
            && discharge_current_mA < MAX_DISCHARGE_CURRENT_mA){
                //do nothing
        }
        else if (discharge_current_mA >= MAX_DISCHARGE_CURRENT_mA              //Discharge current is too high
            || ISL_RegData[Status] & 0b00000110){       //ISL sets discharge OC or short circuit flag
                ISL_SetSpecificBits(ISL.ENABLE_DISCHARGE_FET, 0);   //Disable discharging
                state = DISCHARGE_OC_LOCKOUT;
        }
        else {                                                  //Trigger released; cell voltage too low; over temp; WKUP status = 1; any other ISL error flag
            ISL_SetSpecificBits(ISL.ENABLE_DISCHARGE_FET, 0);   //Disable discharging
            state = IDLE;
        }
        
        
}

void dischargeOClockout(void){
    Set_LED_RGB(0b100); //Red LED
    ISL_Write_Register(FETControl, 0b01000000); //Enable output load monitoring so we can tell when load is removed, also make sure charge and discharge FETs are off
    if (detect != TRIGGER                                      //Trigger isn't pulled
        && ISL_GetSpecificBits(ISL.LOAD_FAIL_STATUS) == 0     //Load is confirmed removed
        && discharge_current_mA == 0){                     //Definitely no discharge current  
            state = IDLE;
            ISL_SetSpecificBits(ISL.VMON_CHECK, 0); //Turn off output load monitoring
    }
}

void error(void){
    //TODO: I should probably add a flag that gets set to specify the type of error encountered. Then we can reference that flag to have an error-specific LED indicator action.
    //Should we just roll all error/lockout states in to this function that will check all possible errors and respond appropriately?
    ISL_SetSpecificBits(ISL.ENABLE_CHARGE_FET, 0);  //Immediately double-check that charge and discharge fets are disabled.
    ISL_SetSpecificBits(ISL.ENABLE_DISCHARGE_FET, 0);
    Set_LED_RGB(0b100); //Red LED
    if (ISL_RegData[Status] == 0){  //Only go back to idle when all error flags are clear
        state = IDLE;
    }
    
    if (sleep_timeout_counter.enable == false){  //If there is an error, start counter (if it isn't already started) before we just go to sleep
        sleep_timeout_counter.value = 0;
        sleep_timeout_counter.enable = true;
    }
    
    if (sleep_timeout_counter.value >  ERROR_SLEEP_TIMEOUT && sleep_timeout_counter.enable == true){    //1876*32ms = 60.032s //If we are in ERROR state for 60 seconds, just go to sleep.
        sleep_timeout_counter.enable = false;
        state = SLEEP;
    }
    
}

void idleWaitTriggerRelease(void){
    ISL_SetSpecificBits(ISL.ENABLE_CHARGE_FET, 0);      //Probably unnecessary. Make sure FETs are off in this state.
    ISL_SetSpecificBits(ISL.ENABLE_DISCHARGE_FET, 0);
    Set_LED_RGB(0b110); //Yellow LED
    if (detect != TRIGGER){
        state = IDLE;
    }
    
}

void main(void)
{
    init();
    
    while (1)
    {
        if (!ISL_GetSpecificBits(ISL.WKPOL)){//If somehow the ISL was reset and so WKPOL isn't correct, reinitialize everything and clear the I2C bus.
            __debug_break();
            I2C1_Init();
            ClearI2CBus();
            I2C_ERROR_FLAGS = 0;    //Clear error flags
        }
        
        ISL_ReadAllCellVoltages();
        ISL_calcCellStats();
        detect = checkDetect();
        isl_int_temp = ISL_GetInternalTemp();
        thermistor_temp = getThermistorTemp(modelnum);
        ISL_Read_Register(Status);      //Get Status register to check for error flags
        discharge_current_mA = dischargeIsense_mA();
        
            //TODO: Add I2C error check here
        
        //It might make sense to handle all error handling state changes here separately. That could cause a timer started in one of the states to not be disabled as expected though.
        
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
                
            case CHARGE_OC_LOCKOUT:
                chargeOClockout();
                break;
                
            case CELL_BALANCE:
                cellBalance();
                break;
                
            case OUTPUT_EN:
                outputEN();
                break;
                
            case DISCHARGE_OC_LOCKOUT:
                dischargeOClockout();
                break;  
                
            case ERROR:
                error();
                break;
                
            case IDLE_WAIT_TRIGGER_RELEASE:
                idleWaitTriggerRelease();
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
        
        
        }


        
        #ifdef __DEBUG
        for (uint8_t i = 0; i < __ISL_NUMBER_OF_REG; i++){
            ISL_Read_Register(i);
        }
        #endif



        
        
    }
}

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
#include "i2c.h"
#include "isl94208.h"
#include "config.h"



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
    return (uint16_t) ((uint32_t)adcval * 2500 / 1024);
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
    return (uint16_t) ((uint32_t)adcval * 2500 * 1000 / 1024 / 2);  //This better maintains precision by doing the multiplication for 2500mV VREF and 1000mA/A in one step as a uint32_t. Then we divide by 1024 ADC steps and the 2mOhm shunt resistor.
}

uint16_t volatile celldelta;
int8_t volatile isl_int_temp;
uint16_t volatile isl_ext_temp;
uint16_t volatile pic_thermistor;
uint16_t volatile discharge_current_isense_mA;

void main(void)
{
    uint16_t sleep_timer_counter = 0;
    I2C_ERROR_FLAGS = 0;
    
    /* Initialize the device */
    SYSTEM_Initialize();
    TMR4_StopTimer();   //Have timer off to start with
    DAC_SetOutput(0);   //Make sure DAC output is 0V = VSS
    
    /* Initialize LEDS */
    redLED = 1;  //Set high to default RED led to off
    greenLED = 1;  //Set high to default GREEN led to off
    blueLED = 1;  //Set high to default BLUE led to off
    TRISB3 = 0; // Set greenLED as output
    TRISA6 = 0; //Set blueLED as output
    TRISA7 = 0; //Set redLED as output
    ANSB3 = 0; //Set Green LED as digital. Red and Blue are always digital.

    /* Immediately start green LED so we know board is alive */
    Set_LED_RGB(0b010);

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
    

    ISL_Read_Register(FeatureSet);  //Read Feature Set register in to storage and record errors if applicable
    if (ISL_GetSpecificBits(ISL.PRESENT) && !I2C_ERROR_FLAGS){    //if config check bit is present and either featset and PRESENT read didn't error out
        Set_LED_RGB(0b001);  //Light blue LED
    }
    else{
        Set_LED_RGB(0b100);  //Light red LED
    }
     __delay_ms(2000);
     Set_LED_RGB(0b000);    // Turn off LED
    __delay_ms(1000);
    
    while (1)
    {
        if (!ISL_GetSpecificBits(ISL.WKPOL)){//If the ISL reset and so WKPOL != 1, fix it.
            Set_LED_RGB(0b110);  //Light yellow LED
            __delay_ms(2000);
            Set_LED_RGB(0b000); //LEDs off
            __delay_ms(2000);
            Set_LED_RGB(0b110);  //Light yellow LED
            __delay_ms(2000);
            Set_LED_RGB(0b000); //LEDs off
            ClearI2CBus();
        }
        
         
//        if (ISL_GetSpecificBits(ISL.WKUP_STATUS) && !I2C_ERROR_FLAGS){  //If WKUP status = 1 and I2C read didn't error out
//            Set_LED_RGB(0b001);  //Light blue LED
//            TMR4_StopTimer();
//            sleep_timer_counter = 0;    //Reset sleep timeout counter if we get any wakeup signal again
//        }
//        else if(I2C_ERROR_FLAGS){
//            Set_LED_RGB(0b110);  //Light yellow LED
//            __delay_ms(100);
//            Set_LED_RGB(0b011);  //Light cyan LED
//            __delay_ms(100);
//            Set_LED_RGB(0b101);  //Light magenta LED
//            __delay_ms(100);
//            ClearI2CBus();
//        }
//        else {  //WKUP_STATUS must be 0, meaning trigger is release and charger isn't connected
//            Set_LED_RGB(0b100);  //Light red LED
//            if (T4CONbits.TMR4ON == 0){
//                TMR4_StartTimer();          //Starting the timer writes to TMR4ON and scaler counters are reset on writes to TxCON
//            }            
//        }

        ISL_ReadAllCellVoltages();
        //uint8_t volatile maxcell = ISL_CalcMaxVoltageCell();
        //uint8_t volatile mincell = ISL_CalcMinVoltageCell();
        celldelta = ISL_CalcCellVoltageDelta();
        isl_int_temp = ISL_GetInternalTemp();
        isl_ext_temp = ISL_GetAnalogOut(AO_EXTTEMP);
        pic_thermistor = readADCmV(ADC_THERMISTOR);
        discharge_current_isense_mA = dischargeIsense_mA();
        
        if (ISL_GetSpecificBits(ISL.WKUP_STATUS) && CellVoltages[ISL_CalcMinVoltageCell()] > 3000 && dischargeIsense_mA() < 10000 && ISL_GetInternalTemp() < 40 && !I2C_ERROR_FLAGS){
            Set_LED_RGB(0b111);     //White LED
            ISL_SetSpecificBits(ISL.ENABLE_DISCHARGE_FET, 1);
            TMR4_StopTimer();
            sleep_timer_counter = 0;    //Reset sleep timeout counter if we get any wakeup signal again
        }
        else if (I2C_ERROR_FLAGS){
           ISL_SetSpecificBits(ISL.ENABLE_DISCHARGE_FET, 0);
           Set_LED_RGB(0b100);  //Red LED
           __debug_break();
        }
        else if (!ISL_GetSpecificBits(ISL.WKUP_STATUS)){
            ISL_SetSpecificBits(ISL.ENABLE_DISCHARGE_FET, 0);
            Set_LED_RGB(0b001); //Blue LED
            if (T4CONbits.TMR4ON == 0){
                TMR4_StartTimer();          //Starting the timer writes to TMR4ON and scaler counters are reset on writes to TxCON
            }
        }
        else{
            ISL_SetSpecificBits(ISL.ENABLE_DISCHARGE_FET, 0);
            Set_LED_RGB(0b110);  //Yellow LED
            __debug_break();
        }
        
        
        
        
        
        if (TMR4_HasOverflowOccured()){
            sleep_timer_counter++;
        }
        
        if (sleep_timer_counter > 156){ //sleep if idle for 5 seconds (32ms TMR4 period * 156 overflow events = 4.992 seconds)
            //go to sleep
            Set_LED_RGB(111);   //Set LED White
            __delay_ms(2000);
            Set_LED_RGB(000);   //Turn off LED
            ISL_SetSpecificBits(ISL.SLEEP, 1);
        }

        
        #ifdef __DEBUG
        for (uint8_t i = 0; i < __ISL_NUMBER_OF_REG; i++){
            ISL_Read_Register(i);
        }
        #endif



        
        
    }
}

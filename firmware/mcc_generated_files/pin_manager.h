/**
  @Generated Pin Manager Header File

  @Company:
    Microchip Technology Inc.

  @File Name:
    pin_manager.h

  @Summary:
    This is the Pin Manager file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  @Description
    This header file provides APIs for driver for .
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.7
        Device            :  PIC16LF1847
        Driver Version    :  2.11
    The generated drivers are tested against the following:
        Compiler          :  XC8 2.31 and above
        MPLAB 	          :  MPLAB X 5.45	
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

#ifndef PIN_MANAGER_H
#define PIN_MANAGER_H

/**
  Section: Included Files
*/

#include <xc.h>

#define INPUT   1
#define OUTPUT  0

#define HIGH    1
#define LOW     0

#define ANALOG      1
#define DIGITAL     0

#define PULL_UP_ENABLED      1
#define PULL_UP_DISABLED     0

// get/set Discharge_Isense_AN0 aliases
#define Discharge_Isense_AN0_TRIS                 TRISAbits.TRISA0
#define Discharge_Isense_AN0_LAT                  LATAbits.LATA0
#define Discharge_Isense_AN0_PORT                 PORTAbits.RA0
#define Discharge_Isense_AN0_ANS                  ANSELAbits.ANSA0
#define Discharge_Isense_AN0_SetHigh()            do { LATAbits.LATA0 = 1; } while(0)
#define Discharge_Isense_AN0_SetLow()             do { LATAbits.LATA0 = 0; } while(0)
#define Discharge_Isense_AN0_Toggle()             do { LATAbits.LATA0 = ~LATAbits.LATA0; } while(0)
#define Discharge_Isense_AN0_GetValue()           PORTAbits.RA0
#define Discharge_Isense_AN0_SetDigitalInput()    do { TRISAbits.TRISA0 = 1; } while(0)
#define Discharge_Isense_AN0_SetDigitalOutput()   do { TRISAbits.TRISA0 = 0; } while(0)
#define Discharge_Isense_AN0_SetAnalogMode()      do { ANSELAbits.ANSA0 = 1; } while(0)
#define Discharge_Isense_AN0_SetDigitalMode()     do { ANSELAbits.ANSA0 = 0; } while(0)

// get/set Thermistor_AN1 aliases
#define Thermistor_AN1_TRIS                 TRISAbits.TRISA1
#define Thermistor_AN1_LAT                  LATAbits.LATA1
#define Thermistor_AN1_PORT                 PORTAbits.RA1
#define Thermistor_AN1_ANS                  ANSELAbits.ANSA1
#define Thermistor_AN1_SetHigh()            do { LATAbits.LATA1 = 1; } while(0)
#define Thermistor_AN1_SetLow()             do { LATAbits.LATA1 = 0; } while(0)
#define Thermistor_AN1_Toggle()             do { LATAbits.LATA1 = ~LATAbits.LATA1; } while(0)
#define Thermistor_AN1_GetValue()           PORTAbits.RA1
#define Thermistor_AN1_SetDigitalInput()    do { TRISAbits.TRISA1 = 1; } while(0)
#define Thermistor_AN1_SetDigitalOutput()   do { TRISAbits.TRISA1 = 0; } while(0)
#define Thermistor_AN1_SetAnalogMode()      do { ANSELAbits.ANSA1 = 1; } while(0)
#define Thermistor_AN1_SetDigitalMode()     do { ANSELAbits.ANSA1 = 0; } while(0)

// get/set RA3 procedures
#define RA3_SetHigh()            do { LATAbits.LATA3 = 1; } while(0)
#define RA3_SetLow()             do { LATAbits.LATA3 = 0; } while(0)
#define RA3_Toggle()             do { LATAbits.LATA3 = ~LATAbits.LATA3; } while(0)
#define RA3_GetValue()              PORTAbits.RA3
#define RA3_SetDigitalInput()    do { TRISAbits.TRISA3 = 1; } while(0)
#define RA3_SetDigitalOutput()   do { TRISAbits.TRISA3 = 0; } while(0)
#define RA3_SetAnalogMode()         do { ANSELAbits.ANSA3 = 1; } while(0)
#define RA3_SetDigitalMode()        do { ANSELAbits.ANSA3 = 0; } while(0)

// get/set ISL_AnalogOut_AN4 aliases
#define ISL_AnalogOut_AN4_TRIS                 TRISAbits.TRISA4
#define ISL_AnalogOut_AN4_LAT                  LATAbits.LATA4
#define ISL_AnalogOut_AN4_PORT                 PORTAbits.RA4
#define ISL_AnalogOut_AN4_ANS                  ANSELAbits.ANSA4
#define ISL_AnalogOut_AN4_SetHigh()            do { LATAbits.LATA4 = 1; } while(0)
#define ISL_AnalogOut_AN4_SetLow()             do { LATAbits.LATA4 = 0; } while(0)
#define ISL_AnalogOut_AN4_Toggle()             do { LATAbits.LATA4 = ~LATAbits.LATA4; } while(0)
#define ISL_AnalogOut_AN4_GetValue()           PORTAbits.RA4
#define ISL_AnalogOut_AN4_SetDigitalInput()    do { TRISAbits.TRISA4 = 1; } while(0)
#define ISL_AnalogOut_AN4_SetDigitalOutput()   do { TRISAbits.TRISA4 = 0; } while(0)
#define ISL_AnalogOut_AN4_SetAnalogMode()      do { ANSELAbits.ANSA4 = 1; } while(0)
#define ISL_AnalogOut_AN4_SetDigitalMode()     do { ANSELAbits.ANSA4 = 0; } while(0)

// get/set RA6 procedures
#define RA6_SetHigh()            do { LATAbits.LATA6 = 1; } while(0)
#define RA6_SetLow()             do { LATAbits.LATA6 = 0; } while(0)
#define RA6_Toggle()             do { LATAbits.LATA6 = ~LATAbits.LATA6; } while(0)
#define RA6_GetValue()              PORTAbits.RA6
#define RA6_SetDigitalInput()    do { TRISAbits.TRISA6 = 1; } while(0)
#define RA6_SetDigitalOutput()   do { TRISAbits.TRISA6 = 0; } while(0)

// get/set RA7 procedures
#define RA7_SetHigh()            do { LATAbits.LATA7 = 1; } while(0)
#define RA7_SetLow()             do { LATAbits.LATA7 = 0; } while(0)
#define RA7_Toggle()             do { LATAbits.LATA7 = ~LATAbits.LATA7; } while(0)
#define RA7_GetValue()              PORTAbits.RA7
#define RA7_SetDigitalInput()    do { TRISAbits.TRISA7 = 1; } while(0)
#define RA7_SetDigitalOutput()   do { TRISAbits.TRISA7 = 0; } while(0)

// get/set SV09Check_AN10 aliases
#define SV09Check_AN10_TRIS                 TRISBbits.TRISB2
#define SV09Check_AN10_LAT                  LATBbits.LATB2
#define SV09Check_AN10_PORT                 PORTBbits.RB2
#define SV09Check_AN10_WPU                  WPUBbits.WPUB2
#define SV09Check_AN10_ANS                  ANSELBbits.ANSB2
#define SV09Check_AN10_SetHigh()            do { LATBbits.LATB2 = 1; } while(0)
#define SV09Check_AN10_SetLow()             do { LATBbits.LATB2 = 0; } while(0)
#define SV09Check_AN10_Toggle()             do { LATBbits.LATB2 = ~LATBbits.LATB2; } while(0)
#define SV09Check_AN10_GetValue()           PORTBbits.RB2
#define SV09Check_AN10_SetDigitalInput()    do { TRISBbits.TRISB2 = 1; } while(0)
#define SV09Check_AN10_SetDigitalOutput()   do { TRISBbits.TRISB2 = 0; } while(0)
#define SV09Check_AN10_SetPullup()          do { WPUBbits.WPUB2 = 1; } while(0)
#define SV09Check_AN10_ResetPullup()        do { WPUBbits.WPUB2 = 0; } while(0)
#define SV09Check_AN10_SetAnalogMode()      do { ANSELBbits.ANSB2 = 1; } while(0)
#define SV09Check_AN10_SetDigitalMode()     do { ANSELBbits.ANSB2 = 0; } while(0)

// get/set RB3 procedures
#define RB3_SetHigh()            do { LATBbits.LATB3 = 1; } while(0)
#define RB3_SetLow()             do { LATBbits.LATB3 = 0; } while(0)
#define RB3_Toggle()             do { LATBbits.LATB3 = ~LATBbits.LATB3; } while(0)
#define RB3_GetValue()              PORTBbits.RB3
#define RB3_SetDigitalInput()    do { TRISBbits.TRISB3 = 1; } while(0)
#define RB3_SetDigitalOutput()   do { TRISBbits.TRISB3 = 0; } while(0)
#define RB3_SetPullup()             do { WPUBbits.WPUB3 = 1; } while(0)
#define RB3_ResetPullup()           do { WPUBbits.WPUB3 = 0; } while(0)
#define RB3_SetAnalogMode()         do { ANSELBbits.ANSB3 = 1; } while(0)
#define RB3_SetDigitalMode()        do { ANSELBbits.ANSB3 = 0; } while(0)

// get/set ChrgDetect_AN7 aliases
#define ChrgDetect_AN7_TRIS                 TRISBbits.TRISB5
#define ChrgDetect_AN7_LAT                  LATBbits.LATB5
#define ChrgDetect_AN7_PORT                 PORTBbits.RB5
#define ChrgDetect_AN7_WPU                  WPUBbits.WPUB5
#define ChrgDetect_AN7_ANS                  ANSELBbits.ANSB5
#define ChrgDetect_AN7_SetHigh()            do { LATBbits.LATB5 = 1; } while(0)
#define ChrgDetect_AN7_SetLow()             do { LATBbits.LATB5 = 0; } while(0)
#define ChrgDetect_AN7_Toggle()             do { LATBbits.LATB5 = ~LATBbits.LATB5; } while(0)
#define ChrgDetect_AN7_GetValue()           PORTBbits.RB5
#define ChrgDetect_AN7_SetDigitalInput()    do { TRISBbits.TRISB5 = 1; } while(0)
#define ChrgDetect_AN7_SetDigitalOutput()   do { TRISBbits.TRISB5 = 0; } while(0)
#define ChrgDetect_AN7_SetPullup()          do { WPUBbits.WPUB5 = 1; } while(0)
#define ChrgDetect_AN7_ResetPullup()        do { WPUBbits.WPUB5 = 0; } while(0)
#define ChrgDetect_AN7_SetAnalogMode()      do { ANSELBbits.ANSB5 = 1; } while(0)
#define ChrgDetect_AN7_SetDigitalMode()     do { ANSELBbits.ANSB5 = 0; } while(0)

/**
   @Param
    none
   @Returns
    none
   @Description
    GPIO and peripheral I/O initialization
   @Example
    PIN_MANAGER_Initialize();
 */
void PIN_MANAGER_Initialize (void);

/**
 * @Param
    none
 * @Returns
    none
 * @Description
    Interrupt on Change Handling routine
 * @Example
    PIN_MANAGER_IOC();
 */
void PIN_MANAGER_IOC(void);



#endif // PIN_MANAGER_H
/**
 End of File
*/
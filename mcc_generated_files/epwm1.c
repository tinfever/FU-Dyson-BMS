/**
  EPWM1 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    epwm1.c

  @Summary
    This is the generated driver implementation file for the EPWM1 driver using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  @Description
    This source file provides implementations for driver APIs for EPWM1.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.7
        Device            :  PIC16LF1847
        Driver Version    :  2.01
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

/**
  Section: Included Files
*/

#include <xc.h>
#include "epwm1.h"

/**
  Section: Macro Declarations
*/

#define PWM1_INITIALIZE_DUTY_VALUE    0

/**
  Section: EPWM Module APIs
*/

void EPWM1_Initialize(void)
{
    // Set the EPWM1 to the options selected in the User Interface
	
	// CCP1M P1A,P1C: active low; P1B,P1D: active low; DC1B 0; P1M single; 
	CCP1CON = 0x0F;    
	
	// CCP1ASE operating; PSS1BD low; PSS1AC low; CCP1AS disabled; 
	ECCP1AS = 0x00;    
	
	// P1RSEN automatic_restart; P1DC 0; 
	PWM1CON = 0x80;    
	
	// STR1D P1D_to_CCP1M; STR1C P1C_to_CCP1M; STR1B P1B_to_port; STR1A P1A_to_CCP1M; STR1SYNC start_at_next; 
	PSTR1CON = 0x1D;    
	
	// CCPR1H 0; 
	CCPR1H = 0x00;    
	
	// CCPR1L 0; 
	CCPR1L = 0x00;    

	// Selecting CCPTMRS0
	CCPTMRS0bits.C1TSEL = 0x0;
}

void EPWM1_LoadDutyValue(uint16_t dutyValue)
{
   // Writing to 8 MSBs of pwm duty cycle in CCPRL register
    CCPR1L = ((dutyValue & 0x03FC)>>2);
    
   // Writing to 2 LSBs of pwm duty cycle in CCPCON register
    CCP1CON = ((uint8_t)(CCP1CON & 0xCF) | ((dutyValue & 0x0003)<<4));
}
/**
 End of File
*/


/**
  EPWM1 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    epwm1.h

  @Summary
    This is the generated driver implementation file for the EPWM1 driver using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  @Description
    This header file provides implementations for driver APIs for EPWM1.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.7
        Device            :  PIC16LF1847
        Driver Version    :  2.01
    The generated drivers are tested against the following:
        Compiler          :  XC8 2.31 and above
        MPLAB             :  MPLAB X 5.45
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

#ifndef EPWM1_H
#define EPWM1_H

/**
  Section: Included Files
*/

#include <xc.h>
#include <stdint.h>

#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif

/**
  Section: EPWM Module APIs
*/

/**
  @Summary
    Initializes the EPWM1

  @Description
    This routine initializes the EPWM1 module.
    This routine must be called before any other EPWM1 routine is called.
    This routine should only be called once during system initialization.

  @Preconditions
    None

  @Param
    None

  @Returns
    None

  @Comment
    

 @Example
    <code>
    uint16_t dutycycle;

    ECCP1_Initialize();
	EPWM1_LoadDutyValue(dutycycle);
    </code>
 */
void EPWM1_Initialize(void);

/**
  @Summary
    Loads 16-bit duty cycle.

  @Description
    This routine loads the 16 bit duty cycle value.

  @Preconditions
    EPWM1_Initialize() function should have been called before calling this function.

  @Param
    Pass 16bit duty cycle value.

  @Returns
    None

  @Example
    <code>
    uint16_t dutycycle;

    EPWM1_Initialize();
    EPWM1_LoadDutyValue(dutycycle);
    </code>
*/
void EPWM1_LoadDutyValue(uint16_t dutyValue);
        
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif

#endif	//EPWM1_H
/**
 End of File
*/

/**
  TMR4 Generated Driver API Header File

  @Company
    Microchip Technology Inc.

  @File Name
    tmr4.h

  @Summary
    This is the generated header file for the TMR4 driver using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  @Description
    This header file provides APIs for TMR4.
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

#ifndef TMR4_H
#define TMR4_H

/**
  Section: Included Files
*/

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif


/**
  Section: Macro Declarations
*/

/**
  Section: TMR4 APIs
*/

/**
  @Summary
    Initializes the TMR4 module.

  @Description
    This function initializes the TMR4 Registers.
    This function must be called before any other TMR4 function is called.

  @Preconditions
    None

  @Param
    None

  @Returns
    None

  @Comment
    

  @Example
    <code>
    main()
    {
        // Initialize TMR4 module
        TMR4_Initialize();

        // Do something else...
    }
    </code>
*/
void TMR4_Initialize(void);

/**
  @Summary
    This function starts the TMR4.

  @Description
    This function starts the TMR4 operation.
    This function must be called after the initialization of TMR4.

  @Preconditions
    Initialize  the TMR4 before calling this function.

  @Param
    None

  @Returns
    None

  @Example
    <code>
    // Initialize TMR4 module

    // Start TMR4
    TMR4_StartTimer();

    // Do something else...
    </code>
*/
void TMR4_StartTimer(void);

/**
  @Summary
    This function stops the TMR4.

  @Description
    This function stops the TMR4 operation.
    This function must be called after the start of TMR4.

  @Preconditions
    Initialize  the TMR4 before calling this function.

  @Param
    None

  @Returns
    None

  @Example
    <code>
    // Initialize TMR4 module

    // Start TMR4
    TMR4_StartTimer();

    // Do something else...

    // Stop TMR4;
    TMR4_StopTimer();
    </code>
*/
void TMR4_StopTimer(void);

/**
  @Summary
    Reads the TMR4 register.

  @Description
    This function reads the TMR4 register value and return it.

  @Preconditions
    Initialize  the TMR4 before calling this function.

  @Param
    None

  @Returns
    This function returns the current value of TMR4 register.

  @Example
    <code>
    // Initialize TMR4 module

    // Start TMR4
    TMR4_StartTimer();

    // Read the current value of TMR4
    if(0 == TMR4_ReadTimer())
    {
        // Do something else...

        // Reload the TMR value
        TMR4_Reload();
    }
    </code>
*/
uint8_t TMR4_ReadTimer(void);

/**
  @Summary
    Writes the TMR4 register.

  @Description
    This function writes the TMR4 register.
    This function must be called after the initialization of TMR4.

  @Preconditions
    Initialize  the TMR4 before calling this function.

  @Param
    timerVal - Value to write into TMR4 register.

  @Returns
    None

  @Example
    <code>
    #define PERIOD 0x80
    #define ZERO   0x00

    while(1)
    {
        // Read the TMR4 register
        if(ZERO == TMR4_ReadTimer())
        {
            // Do something else...

            // Write the TMR4 register
            TMR4_WriteTimer(PERIOD);
        }

        // Do something else...
    }
    </code>
*/
void TMR4_WriteTimer(uint8_t timerVal);

/**
  @Summary
    Load value to Period Register.

  @Description
    This function writes the value to PR4 register.
    This function must be called after the initialization of TMR4.

  @Preconditions
    Initialize  the TMR4 before calling this function.

  @Param
    periodVal - Value to load into TMR4 register.

  @Returns
    None

  @Example
    <code>
    #define PERIOD1 0x80
    #define PERIOD2 0x40
    #define ZERO    0x00

    while(1)
    {
        // Read the TMR4 register
        if(ZERO == TMR4_ReadTimer())
        {
            // Do something else...

            if(flag)
            {
                flag = 0;

                // Load Period 1 value
                TMR4_LoadPeriodRegister(PERIOD1);
            }
            else
            {
                 flag = 1;

                // Load Period 2 value
                TMR4_LoadPeriodRegister(PERIOD2);
            }
        }

        // Do something else...
    }
    </code>
*/
void TMR4_LoadPeriodRegister(uint8_t periodVal);

/**
  @Summary
    Boolean routine to poll or to check for the match flag on the fly.

  @Description
    This function is called to check for the timer match flag.
    This function is used in timer polling method.

  @Preconditions
    Initialize  the TMR4 module before calling this routine.

  @Param
    None

  @Returns
    true - timer match has occurred.
    false - timer match has not occurred.

  @Example
    <code>
    while(1)
    {
        // check the match flag
        if(TMR4_HasOverflowOccured())
        {
            // Do something else...

            // Reload the TMR4 value
            TMR4_Reload();
        }
    }
    </code>
*/
bool TMR4_HasOverflowOccured(void);

 #ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif

#endif // TMR4_H
/**
 End of File
*/


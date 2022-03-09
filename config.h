/* 
 * File:   config.h
 * Author: user
 *
 * Created on February 25, 2022, 2:45 PM
 */

#ifndef CONFIG_H
#define	CONFIG_H



#define redLED RA7
#define greenLED RB3
#define blueLED RA6

#define SDA RB1
#define SCL RB4
#define TRIS_SDA TRISB1
#define TRIS_SCL TRISB4
#define ANS_SDA ANSB1
#define ANS_SCL ANSB4

#define ISL_ADDR 0x50

/*ADC Channels, Defined in adc.h
     Discharge_Isense_AN0 =     0x0,
    Thermistor_AN1 =            0x1,
    ISL_AnalogOut_AN4 =         0x4,
    channel_Temp =              0x1D,
    channel_DAC =               0x1E,
    channel_FVR =               0x1F
 */

#define ADC_DISCHARGE_ISENSE 0x0
#define ADC_THERMISTOR 0x1
#define ADC_ISL_OUT 0x4
#define ADC_PIC_INT_TEMP 0x1D
#define ADC_PIC_DAC 0x1E
#define ADC_PIC_FVR 0x1F
#define ADC_CHRG_TRIG_DETECT 0x07
#define ADC_SV09CHECK 0x0A

const uint8_t MAX_CHARGE_TEMP_C = 50;           //Celsius
const uint8_t MAX_DISCHARGE_TEMP_C = 60;        //Celsius






















#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* CONFIG_H */


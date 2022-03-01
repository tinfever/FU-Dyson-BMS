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

#define ADC_DISCHARGE_ISENSE 0x0
#define ADC_THERMISTOR 0x1
#define ADC_ISL_OUT 0x4



























#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* CONFIG_H */


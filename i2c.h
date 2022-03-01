/*
 * File: i2c.h
 * Author: DavidAlfa at EEVblog
 * 
 * Based on Armstrong Subero's code:
 * https://github.com/Apress/programming-pic-microcontrollers-w-xc8/tree/master/Chapter%209/EEPROM/24_I2C1.X
 *
 */

#ifndef _I2C_H_
#define _I2C_H_


/*
 * Warning: You need to properly configure IO pins before calling I2C functions
 * Some devices need disabling analog mode for the SPI pins, others have PPS... 
 * 
 * 
 * You can disable parts if not all I2C modules are used
 * The selected code will only be enabled if the module exists in the device
 */

/* Enable I2C code for SSP1 */
#define ENABLE_I2C_SSP1

/* Enable I2C code for SSP2 */
//#define ENABLE_I2C_SSP2

/* Enable I2C code for SSP (Older devices) */
//#define ENABLE_I2C_SSP

typedef enum{
    I2C_OK  = 0,
    I2C_NO_ACK = 1,
    I2C_COLLISION = 2,
    I2C_OVERFLOW = 4,
    I2C_BUS_COLLISION_IF = 8,
    I2C_TIMEOUT = 16
} i2c_result_t;

/* Enable code if device has SSP1 module and it's user-enabled */
#if defined ENABLE_I2C_SSP1 && defined SSP1BUF
void I2C1_Init(void);
void I2C1_Enable(void);
void I2C1_Disable(void);
i2c_result_t I2C1_ReadMemory(unsigned char devAddr, unsigned char reg, unsigned char *dest, unsigned char size);
i2c_result_t I2C1_WriteMemory(unsigned char devAddr, unsigned char reg, unsigned char *src, unsigned char size);
i2c_result_t I2C1_Read(unsigned char devAddr,unsigned char *dest, unsigned char size);
i2c_result_t I2C1_Write(unsigned char devAddr, unsigned char *src, unsigned char size);
#endif

/* Prevent legacy use of SSP1 */
#if defined ENABLE_I2C_SSP && defined SSP1BUF
#error Your device has SSP1, please use ENABLE_I2C_SSP1 instead

/* Enable code if device has SSP module and it's user-enabled */
#elif defined ENABLE_I2C_SSP && defined SSPBUF
void I2C_Init(void);
void I2C_Enable(void);
void I2C_Disable(void);
i2c_result_t I2C_ReadMemory(unsigned char devAddr, unsigned char reg, unsigned char *dest, unsigned char size);
i2c_result_t I2C_WriteMemory(unsigned char devAddr, unsigned char reg, unsigned char *src, unsigned char size);
i2c_result_t I2C_Read(unsigned char devAddr,unsigned char *dest, unsigned char size);
i2c_result_t I2C_Write(unsigned char devAddr, unsigned char *src, unsigned char size);
#endif

/* Enable code if device has SSP2 module and it's user-enabled */
#if defined ENABLE_I2C_SSP2 && defined SSP2BUF
void I2C2_Init(void);
void I2C2_Enable(void);
void I2C2_Disable(void);
i2c_result_t I2C2_ReadMemory(unsigned char devAddr, unsigned char reg, unsigned char *dest, unsigned char size);
i2c_result_t I2C2_WriteMemory(unsigned char devAddr, unsigned char reg, unsigned char *src, unsigned char size);
i2c_result_t I2C2_Read(unsigned char devAddr,unsigned char *dest, unsigned char size);
i2c_result_t I2C2_Write(unsigned char devAddr, unsigned char *src, unsigned char size);
#endif

#endif
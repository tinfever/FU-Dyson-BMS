/*
 * File: i2c.x
 * Author: DavidAlfa at EEVblog
 * 
 * Based on Armstrong Subero's code:
 * https://github.com/Apress/programming-pic-microcontrollers-w-xc8/tree/master/Chapter%209/EEPROM/24_I2C1.X
 *
 */

#include "mcc_generated_files/mcc.h"
#include "i2c.h"


/* Enable code if device has SSP module and it's user-enabled */
#if defined ENABLE_I2C_SSP1 && defined SSP1BUF
/***************************************************************************************
 Private functions
***************************************************************************************/
static i2c_result_t _I2C1_WriteReg(unsigned char devAddr, unsigned char reg);
static void _I2C1_Write(unsigned char data);
static unsigned char _I2C1_Read(void);
static void _I2C1_StartBit(void);
static void _I2C1_StopBit(void);
static void _I2C1_ACK(void);
static void _I2C1_NACK(void);
static i2c_result_t _I2C1_GetError(void);

/* Disable write start flag, to avoid sending repeated start condition on memory writes */
static unsigned char _I2C1_No_WrStart = 0;

/* Initialize timeout variable */
static uint8_t timeout_counter = 0;

/***************************************************************************************
 Check for error conditions that preclude SSPIF from being set
***************************************************************************************/

bool I2C1_CheckErrorCondition(){
    if(SSP1CON2bits.ACKSTAT){        
        return true;
    }
    if(SSP1CON1bits.WCOL){
        return true;
    }
    if(SSP1CON1bits.SSPOV){
        return true;
    }
    if (PIR2bits.BCL1IF){
        return true;
    }
    if (timeout_counter > 250){
        return true;
    }
    timeout_counter++;  //Approximately 117 cycles between counter increments = ~914uS timeout period at 32MHz clock
    return false;
    
}

/***************************************************************************************
 Init and enable I2C1 peripheral
***************************************************************************************/
void I2C1_Init(void)
{
    SSP1CON1bits.SSPM=0x08;         // I2C Master mode, clock = Fosc/(4 * (SSPADD+1))
    SSP1CON1bits.SSPEN=1;           // enable MSSP port
    //SSP1ADD = 0x4F;                 // set Baud rate clock divider (100KHz for 32MHz clock)
    SSP1ADD = 0x13;                 // set Baud rate clock divider (400KHz for 32MHz clock)
    __delay_us(5);                  // let everything settle.
}

/***************************************************************************************
 Enable I2C1 peripheral
***************************************************************************************/
void I2C1_Enable(void)
{
    SSP1CON1bits.SSPEN=1;           // Enable MSSP port
}

/***************************************************************************************
 Disable I2C1 peripheral
***************************************************************************************/
void I2C1_Disable(void)
{
    SSP1CON1bits.SSPEN=0;           // disable MSSP port
}

/***************************************************************************************
  Select I2C reg, read data
***************************************************************************************/
i2c_result_t I2C1_ReadMemory(unsigned char devAddr, unsigned char reg, unsigned char *dest, unsigned char size)
{
    i2c_result_t res;
    
    res = _I2C1_WriteReg(devAddr, reg);     // Send reg address
    if(res) return res;                     // Check for errors    
    return I2C1_Read(devAddr, dest, size);  // Read memory
}

/***************************************************************************************
  Select I2C reg, write data
***************************************************************************************/
i2c_result_t I2C1_WriteMemory(unsigned char devAddr, unsigned char reg, unsigned char *src, unsigned char size)
{
    i2c_result_t res;
    
    res = _I2C1_WriteReg(devAddr, reg);     // Send reg address
    if(res) return res;                     // Check for errors    
    _I2C1_No_WrStart = 1;                   // Don't send repeated start
    return I2C1_Write(devAddr, src, size);  // Write memory
    
}

/***************************************************************************************
  Read I2C data from device
***************************************************************************************/
i2c_result_t I2C1_Read(unsigned char devAddr,unsigned char *dest, unsigned char size)
{
    i2c_result_t res;
    
    _I2C1_StartBit();               // send start bit
    res = _I2C1_GetError();
    if(res) return res;             // Check for errors
    _I2C1_Write(devAddr | 1);       // send device address (RW bit = Read)
    res = _I2C1_GetError();
    if(res) return res;             // Check for errors
    while (size--){
        *dest++ = _I2C1_Read();     // now we read the data        
        res = _I2C1_GetError();
        if(res) return res;         // Check for errors
    }
    _I2C1_NACK();                   // send a the NAK to tell the device we don't want any more data
    res = _I2C1_GetError();
    if(res) return res;             // Check for errors
    _I2C1_StopBit();                // Send the stop bit
    return _I2C1_GetError();    
}

/***************************************************************************************
  Write I2C data from device
***************************************************************************************/
i2c_result_t I2C1_Write(unsigned char devAddr, unsigned char *src, unsigned char size)
{
    i2c_result_t res;
    
    if(_I2C1_No_WrStart){
        _I2C1_No_WrStart=0;
    }
    else{
        _I2C1_StartBit();           // Send start bit
        res = _I2C1_GetError();
        if(res) return res;         // Check for errors        
        _I2C1_Write(devAddr);       // send device address (RW bit = Write)
        res = _I2C1_GetError();
        if(res) return res;         // Check for errors
    }
    while (size--){
        _I2C1_Write(*src++);        // send data
        res = _I2C1_GetError();
        if(res) return res;         // Check for errors
    }
    _I2C1_StopBit();                // send the stop bit
    return _I2C1_GetError();    
}

/***************************************************************************************
  Start, send register address
***************************************************************************************/
static i2c_result_t _I2C1_WriteReg(unsigned char devAddr, unsigned char reg)
{
    i2c_result_t res;
    
    _I2C1_StartBit();               // send start bit
    res = _I2C1_GetError();    
    if(res) return res;             // Check for errors
    _I2C1_Write(devAddr);           // send device address (RW bit = Write)
    res = _I2C1_GetError();
    if(res) return res;             // Check for errors    
    _I2C1_Write(reg);               // send register address
    return _I2C1_GetError();
}
    
/***************************************************************************************
   Send one byte
***************************************************************************************/
static void _I2C1_Write(unsigned char data)
{
    PIR1bits.SSP1IF=0;              // clear SSP interrupt bit
    SSP1BUF = data;                 // send data
    timeout_counter = 0;
    while(!PIR1bits.SSP1IF && !I2C1_CheckErrorCondition());        // Wait for interrupt flag to go high indicating transmission is complete or for an error condition to become present
}

/***************************************************************************************
  Read one byte
***************************************************************************************/
static unsigned char _I2C1_Read(void)
{
    PIR1bits.SSP1IF=0;              // clear SSP interrupt bit
    SSP1CON2bits.RCEN=1;            // set the receive enable bit to initiate a read of 8 bits
    timeout_counter = 0;
    while(!PIR1bits.SSP1IF && !I2C1_CheckErrorCondition());        // Wait for interrupt flag to go high indicating transmission is complete
    return (SSP1BUF);               // Data is now in the SSPBUF so return that value
}

/***************************************************************************************
   Send start bit
***************************************************************************************/
static void _I2C1_StartBit(void)
{
    PIR1bits.SSP1IF=0;              // clear SSP interrupt bit
    SSP1CON2bits.SEN=1;             // send start bit
    timeout_counter = 0;
    while(!PIR1bits.SSP1IF && !I2C1_CheckErrorCondition());        // Wait for the SSPIF bit to go back high before we load the data buffer
}

/***************************************************************************************
   Send stop bit
***************************************************************************************/
static void _I2C1_StopBit(void)
{
    PIR1bits.SSP1IF=0;              // clear SSP interrupt bit
    SSP1CON2bits.PEN=1;             // send stop bit
    timeout_counter = 0;
    while(!PIR1bits.SSP1IF && !I2C1_CheckErrorCondition());        // Wait for interrupt flag to go high indicating transmission is complete
}


/***************************************************************************************
   Send ACK bit
***************************************************************************************/
static void _I2C1_ACK(void)
{
   PIR1bits.SSP1IF=0;               // clear SSP interrupt bit
   SSP1CON2bits.ACKDT=0;            // clear the Acknowledge Data Bit - this means we are sending an Acknowledge or 'ACK'
   SSP1CON2bits.ACKEN=1;            // set the ACK enable bit to initiate transmission of the ACK bit
   timeout_counter = 0;
   while(!PIR1bits.SSP1IF && !I2C1_CheckErrorCondition());         // Wait for interrupt flag to go high indicating transmission is complete
}

/***************************************************************************************
   Send NAK bit
***************************************************************************************/
static void _I2C1_NACK(void)
{
    PIR1bits.SSP1IF=0;              // clear SSP interrupt bit
    SSP1CON2bits.ACKDT=1;           // set the Acknowledge Data Bit- this means we are sending a No-Ack or 'NAK'
    SSP1CON2bits.ACKEN=1;           // set the ACK enable bit to initiate transmission of the NACK bit
    timeout_counter = 0;
    while(!PIR1bits.SSP1IF && !I2C1_CheckErrorCondition());        // Wait for interrupt flag to go high indicating transmission is complete
}

/***************************************************************************************
  Get and parse errors. Send Stop bit on error
***************************************************************************************/
static i2c_result_t _I2C1_GetError(void){    
    
    if(SSP1CON2bits.ACKSTAT){        
        _I2C1_StopBit();
        return I2C_NO_ACK;
    }
    if(SSP1CON1bits.WCOL){
        SSP1CON1bits.WCOL = 0;
        _I2C1_StopBit();
        return I2C_COLLISION;
    }
    if(SSP1CON1bits.SSPOV){
        SSP1CON1bits.SSPOV = 0;        
        _I2C1_StopBit();
        return I2C_OVERFLOW;
    }
    if (PIR2bits.BCL1IF){
        PIR2bits.BCL1IF = 0;
        _I2C1_StopBit();
        return I2C_BUS_COLLISION_IF;
    }    
    if (timeout_counter > 250){
        _I2C1_StopBit();
        return I2C_TIMEOUT;
    }
    return I2C_OK;
}
#endif

/* Prevent legacy use of SSP1 */
#if defined ENABLE_I2C_SSP && defined SSP1BUF
#error Your device has SSP1, use ENABLE_I2C_SSP1 instead

/* Enable code if device has SSP module and it's user-enabled */
#elif defined ENABLE_I2C_SSP && defined SSPBUF
/***************************************************************************************
 Private functions
***************************************************************************************/
static i2c_result_t _I2C_WriteReg(unsigned char devAddr, unsigned char reg);
static void _I2C_Write(unsigned char data);
static unsigned char _I2C_Read(void);
static void _I2C_StartBit(void);
static void _I2C_StopBit(void);
static void _I2C_ACK(void);
static void _I2C_NACK(void);
static i2c_result_t _I2C_GetError(void);

/* Disable write start flag, to avoid sending repeated start condition on memory writes */
static unsigned char _I2C_No_WrStart = 0;

/***************************************************************************************
 Init and enable I2C peripheral
***************************************************************************************/
void I2C_Init(void)
{
    SSPCON1bits.SSPM=0x08;          // I2C Master mode, clock = Fosc/(4 * (SSPADD+1))
    SSPCON1bits.SSPEN=1;            // enable MSSP port
    SSPADD = 0x4F;                  // set Baud rate clock divider (100KHz for 32MHz clock)
    __delay_us(5);                  // let everything settle.
}

/***************************************************************************************
 Enable I2C peripheral
***************************************************************************************/
void I2C_Enable(void)
{
    SSPCON1bits.SSPEN=1;            // Enable MSSP port
}

/***************************************************************************************
 Disable I2C peripheral
***************************************************************************************/
void I2C_Disable(void)
{
    SSPCON1bits.SSPEN=0;            // disable MSSP port
}

/***************************************************************************************
  Select I2C reg, read data
***************************************************************************************/
i2c_result_t I2C_ReadMemory(unsigned char devAddr, unsigned char reg, unsigned char *dest, unsigned char size)
{
    i2c_result_t res;
    
    res = _I2C_WriteReg(devAddr, reg);      // Send reg address
    if(res) return res;                     // Check for errors    
    return I2C_Read(devAddr, dest, size);   // Read memory
}

/***************************************************************************************
  Select I2C reg, write data
***************************************************************************************/
i2c_result_t I2C_WriteMemory(unsigned char devAddr, unsigned char reg, unsigned char *src, unsigned char size)
{
    i2c_result_t res;
    
    res = _I2C_WriteReg(devAddr, reg);      // Send reg address
    if(res) return res;                     // Check for errors    
    _I2C_No_WrStart = 1;                    // Don't send repeated start
    return I2C_Write(devAddr, src, size);   // Write memory
    
}

/***************************************************************************************
  Read I2C data from device
***************************************************************************************/
i2c_result_t I2C_Read(unsigned char devAddr,unsigned char *dest, unsigned char size)
{
    i2c_result_t res;
    
    _I2C_StartBit();                // send start bit
    res = _I2C_GetError();
    if(res) return res;             // Check for errors
    _I2C_Write(devAddr | 1);        // send device address (RW bit = Read)
    res = _I2C_GetError();
    if(res) return res;             // Check for errors
    while (size--){
        *dest++ = _I2C_Read();      // now we read the data        
        res = _I2C_GetError();
        if(res) return res;         // Check for errors
    }
    _I2C_NACK();                    // send a the NAK to tell the device we don't want any more data
    res = _I2C_GetError();
    if(res) return res;             // Check for errors
    _I2C_StopBit();                 // Send the stop bit
    return _I2C_GetError();    
}

/***************************************************************************************
  Write I2C data from device
***************************************************************************************/
i2c_result_t I2C_Write(unsigned char devAddr, unsigned char *src, unsigned char size)
{
    i2c_result_t res;
    
    if(_I2C_No_WrStart){
        _I2C_No_WrStart=0;
    }
    else{
        _I2C_StartBit();            // Send start bit
        res = _I2C_GetError();
        if(res) return res;         // Check for errors        
        _I2C_Write(devAddr);        // send device address (RW bit = Write)
        res = _I2C_GetError();
        if(res) return res;         // Check for errors
    }
    while (size--){
        _I2C_Write(*src++);         // send data
        res = _I2C_GetError();
        if(res) return res;         // Check for errors
    }
    _I2C_StopBit();                 // send the stop bit
    return _I2C_GetError();    
}

/***************************************************************************************
  Start, send register address
***************************************************************************************/
static i2c_result_t _I2C_WriteReg(unsigned char devAddr, unsigned char reg)
{
    i2c_result_t res;
    
    _I2C_StartBit();                // send start bit
    res = _I2C_GetError();    
    if(res) return res;             // Check for errors
    _I2C_Write(devAddr);            // send device address (RW bit = Write)
    res = _I2C_GetError();
    if(res) return res;             // Check for errors    
    _I2C_Write(reg);                // send register address
    return _I2C_GetError();
}
    
/***************************************************************************************
   Send one byte
***************************************************************************************/
static void _I2C_Write(unsigned char data)
{
    PIR1bits.SSPIF=0;               // clear SSP interrupt bit
    SSPBUF = data;                  // send data
    while(!PIR1bits.SSPIF);         // Wait for interrupt flag to go high indicating transmission is complete
}

/***************************************************************************************
  Read one byte
***************************************************************************************/
static unsigned char _I2C_Read(void)
{
    PIR1bits.SSPIF=0;               // clear SSP interrupt bit
    SSPCON2bits.RCEN=1;             // set the receive enable bit to initiate a read of 8 bits
    while(!PIR1bits.SSPIF);         // Wait for interrupt flag to go high indicating transmission is complete
    return (SSPBUF);                // Data is now in the SSPBUF so return that value
}

/***************************************************************************************
   Send start bit
***************************************************************************************/
static void _I2C_StartBit(void)
{
    PIR1bits.SSPIF=0;               // clear SSP interrupt bit
    SSPCON2bits.SEN=1;              // send start bit
    while(!PIR1bits.SSPIF);         // Wait for the SSPIF bit to go back high before we load the data buffer
}

/***************************************************************************************
   Send stop bit
***************************************************************************************/
static void _I2C_StopBit(void)
{
    PIR1bits.SSPIF=0;               // clear SSP interrupt bit
    SSPCON2bits.PEN=1;              // send stop bit
    while(!PIR1bits.SSPIF);         // Wait for interrupt flag to go high indicating transmission is complete
}


/***************************************************************************************
   Send ACK bit
***************************************************************************************/
static void _I2C_ACK(void)
{
   PIR1bits.SSPIF=0;                // clear SSP interrupt bit
   SSPCON2bits.ACKDT=0;             // clear the Acknowledge Data Bit - this means we are sending an Acknowledge or 'ACK'
   SSPCON2bits.ACKEN=1;             // set the ACK enable bit to initiate transmission of the ACK bit
   while(!PIR1bits.SSPIF);          // Wait for interrupt flag to go high indicating transmission is complete
}

/***************************************************************************************
   Send NAK bit
***************************************************************************************/
static void _I2C_NACK(void)
{
    PIR1bits.SSPIF=0;               // clear SSP interrupt bit
    SSPCON2bits.ACKDT=1;            // set the Acknowledge Data Bit- this means we are sending a No-Ack or 'NAK'
    SSPCON2bits.ACKEN=1;            // set the ACK enable bit to initiate transmission of the NACK bit
    while(!PIR1bits.SSPIF);         // Wait for interrupt flag to go high indicating transmission is complete
}

/***************************************************************************************
  Get and parse errors. Send Stop bit on error
***************************************************************************************/
static i2c_result_t _I2C_GetError(void){    
    
    if(SSPCON2bits.ACKSTAT){        
        _I2C_StopBit();
        return I2C_NO_ACK;
    }
    if(SSPCON1bits.WCOL){
        SSPCON1bits.WCOL = 0;
        _I2C_StopBit();
        return I2C_COLLISION;
    }
    if(SSPCON1bits.SSPOV){
        SSPCON1bits.SSPOV = 0;        
        _I2C_StopBit();
        return I2C_OVERFLOW;
    }    
    return I2C_OK;
}
#endif


/* Enable code if device has SSP2 module and it's user-enabled */
#if defined ENABLE_I2C_SSP2 && defined SSP2BUF
/***************************************************************************************
 Private functions
***************************************************************************************/
static i2c_result_t _I2C2_WriteReg(unsigned char devAddr, unsigned char reg);
static void _I2C2_Write(unsigned char data);
static unsigned char _I2C2_Read(void);
static void _I2C2_StartBit(void);
static void _I2C2_StopBit(void);
static void _I2C2_ACK(void);
static void _I2C2_NACK(void);
static i2c_result_t _I2C2_GetError(void);

/* Disable write start flag, to avoid sending repeated start condition on memory writes */
static unsigned char _I2C2_No_WrStart = 0;

/***************************************************************************************
 Init and enable I2C2 peripheral
***************************************************************************************/
void I2C2_Init(void)
{
    SSP2CON1bits.SSPM=0x08;         // I2C Master mode, clock = Fosc/(4 * (SSPADD+1))
    SSP2CON1bits.SSPEN=1;           // enable MSSP port
    SSP2ADD = 0x4F;                 // set Baud rate clock divider (100KHz for 32MHz clock)
    __delay_us(5);                  // let everything settle.
}

/***************************************************************************************
 Enable I2C2 peripheral
***************************************************************************************/
void I2C2_Enable(void)
{
    SSP2CON1bits.SSPEN=1;           // Enable MSSP port
}

/***************************************************************************************
 Disable I2C2 peripheral
***************************************************************************************/
void I2C2_Disable(void)
{
    SSP2CON1bits.SSPEN=0;           // disable MSSP port
}

/***************************************************************************************
  Select I2C reg, read data
***************************************************************************************/
i2c_result_t I2C2_ReadMemory(unsigned char devAddr, unsigned char reg, unsigned char *dest, unsigned char size)
{
    i2c_result_t res;
    
    res = _I2C2_WriteReg(devAddr, reg);     // Send reg address
    if(res) return res;                     // Check for errors    
    return I2C2_Read(devAddr, dest, size);  // Read memory
}

/***************************************************************************************
  Select I2C reg, write data
***************************************************************************************/
i2c_result_t I2C2_WriteMemory(unsigned char devAddr, unsigned char reg, unsigned char *src, unsigned char size)
{
    i2c_result_t res;
    
    res = _I2C2_WriteReg(devAddr, reg);     // Send reg address
    if(res) return res;                     // Check for errors    
    _I2C2_No_WrStart = 1;                   // Don't send repeated start
    return I2C2_Write(devAddr, src, size);  // Write memory
    
}

/***************************************************************************************
  Read I2C data from device
***************************************************************************************/
i2c_result_t I2C2_Read(unsigned char devAddr,unsigned char *dest, unsigned char size)
{
    i2c_result_t res;
    
    _I2C2_StartBit();               // send start bit
    res = _I2C2_GetError();
    if(res) return res;             // Check for errors
    _I2C2_Write(devAddr | 1);       // send device address (RW bit = Read)
    res = _I2C2_GetError();
    if(res) return res;             // Check for errors
    while (size--){
        *dest++ = _I2C2_Read();     // now we read the data        
        res = _I2C2_GetError();
        if(res) return res;         // Check for errors
    }
    _I2C2_NACK();                   // send a the NAK to tell the device we don't want any more data
    res = _I2C2_GetError();
    if(res) return res;             // Check for errors
    _I2C2_StopBit();                // Send the stop bit
    return _I2C2_GetError();    
}

/***************************************************************************************
  Write I2C data from device
***************************************************************************************/
i2c_result_t I2C2_Write(unsigned char devAddr, unsigned char *src, unsigned char size)
{
    i2c_result_t res;
    
    if(_I2C2_No_WrStart){
        _I2C2_No_WrStart=0;
    }
    else{
        _I2C2_StartBit();           // Send start bit
        res = _I2C2_GetError();
        if(res) return res;         // Check for errors        
        _I2C2_Write(devAddr);       // send device address (RW bit = Write)
        res = _I2C2_GetError();
        if(res) return res;         // Check for errors
    }
    while (size--){
        _I2C2_Write(*src++);        // send data
        res = _I2C2_GetError();
        if(res) return res;         // Check for errors
    }
    _I2C2_StopBit();                // send the stop bit
    return _I2C2_GetError();    
}

/***************************************************************************************
  Start, send register address
***************************************************************************************/
static i2c_result_t _I2C2_WriteReg(unsigned char devAddr, unsigned char reg)
{
    i2c_result_t res;
    
    _I2C2_StartBit();               // send start bit
    res = _I2C2_GetError();    
    if(res) return res;             // Check for errors
    _I2C2_Write(devAddr);           // send device address (RW bit = Write)
    res = _I2C2_GetError();
    if(res) return res;             // Check for errors    
    _I2C2_Write(reg);               // send register address
    return _I2C2_GetError();
}
    
/***************************************************************************************
   Send one byte
***************************************************************************************/
static void _I2C2_Write(unsigned char data)
{
    PIR4bits.SSP2IF=0;              // clear SSP interrupt bit
    SSP2BUF = data;                 // send data
    while(!PIR4bits.SSP2IF);        // Wait for interrupt flag to go high indicating transmission is complete
}

/***************************************************************************************
  Read one byte
***************************************************************************************/
static unsigned char _I2C2_Read(void)
{
    PIR4bits.SSP2IF=0;              // clear SSP interrupt bit
    SSP2CON2bits.RCEN=1;            // set the receive enable bit to initiate a read of 8 bits
    while(!PIR4bits.SSP2IF);        // Wait for interrupt flag to go high indicating transmission is complete
    return (SSP2BUF);               // Data is now in the SSPBUF so return that value
}

/***************************************************************************************
   Send start bit
***************************************************************************************/
static void _I2C2_StartBit(void)
{
    PIR4bits.SSP2IF=0;              // clear SSP interrupt bit
    SSP2CON2bits.SEN=1;             // send start bit
    while(!PIR4bits.SSP2IF);        // Wait for the SSPIF bit to go back high before we load the data buffer
}

/***************************************************************************************
   Send stop bit
***************************************************************************************/
static void _I2C2_StopBit(void)
{
    PIR4bits.SSP2IF=0;              // clear SSP interrupt bit
    SSP2CON2bits.PEN=1;             // send stop bit
    while(!PIR4bits.SSP2IF);        // Wait for interrupt flag to go high indicating transmission is complete
}


/***************************************************************************************
   Send ACK bit
***************************************************************************************/
static void _I2C2_ACK(void)
{
   PIR4bits.SSP2IF=0;               // clear SSP interrupt bit
   SSP2CON2bits.ACKDT=0;            // clear the Acknowledge Data Bit - this means we are sending an Acknowledge or 'ACK'
   SSP2CON2bits.ACKEN=1;            // set the ACK enable bit to initiate transmission of the ACK bit
   while(!PIR4bits.SSP2IF);         // Wait for interrupt flag to go high indicating transmission is complete
}

/***************************************************************************************
   Send NAK bit
***************************************************************************************/
static void _I2C2_NACK(void)
{
    PIR4bits.SSP2IF=0;              // clear SSP interrupt bit
    SSP2CON2bits.ACKDT=1;           // set the Acknowledge Data Bit- this means we are sending a No-Ack or 'NAK'
    SSP2CON2bits.ACKEN=1;           // set the ACK enable bit to initiate transmission of the NACK bit
    while(!PIR4bits.SSP2IF);        // Wait for interrupt flag to go high indicating transmission is complete
}

/***************************************************************************************
  Get and parse errors. Send Stop bit on error
***************************************************************************************/
static i2c_result_t _I2C2_GetError(void){    
    
    if(SSP2CON2bits.ACKSTAT){        
        _I2C2_StopBit();
        return I2C_NO_ACK;
    }
    if(SSP2CON1bits.WCOL){
        SSP2CON1bits.WCOL = 0;
        _I2C2_StopBit();
        return I2C_COLLISION;
    }
    if(SSP2CON1bits.SSPOV){
        SSP2CON1bits.SSPOV = 0;        
        _I2C2_StopBit();
        return I2C_OVERFLOW;
    }    
    return I2C_OK;
}
#endif
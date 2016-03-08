/*
    The I2C driver.
*/


#include "i2c.h"
#include <targets/LPC2000.h>


/**
    Initializes the I2C driver.
*/
void i2c_Init(void)
{
    PINSEL0 |= 0x50; 
    I2SCLL = I2SCLH = 400;	// 75kBits @ 58,98 <MHz
    I2CONSET = 0x40;		// enable I2C bus interface
}

/**
    Handles timeouts and interrupts.
*/
static void wait_for_SI(void)
{
    unsigned long timeout = 1600000;
    I2CONCLR = 8;
    while (timeout-- && !(I2CONSET & 8));
}


/**
    Sends the START condition.
    @param addr The address.
*/
int i2c_Start(int addr)
{
    I2CONCLR = 0x14;								
    I2CONSET = 0x28;								
    wait_for_SI();
    I2CONCLR = 0x20;
                                                                    
    if (I2STAT > 0x10) {
        return -1;
    }

    I2DAT = addr;								
    wait_for_SI();
    return (I2STAT != 0x40 && I2STAT != 0x18);
}


/**
    Writes on the I2C bus.
    @param buf The buffer to be written.
    @param count How many times to try.
*/
int i2c_Write(unsigned char *buf, unsigned count)
{
    while (count--) {
        I2DAT = *buf++;						
        wait_for_SI();
        if (I2STAT != 0x28) {
            return 1;		
	}
    }
    return 0;
}

/**
    Reads from the I2C bus.
*/
int i2c_Read(void)
{
    I2CONSET = AA; 
    wait_for_SI();
    if (I2STAT != 0x50 && I2STAT != 0x40) {
        return -1;
    } else {
      return I2DAT;
    }
}

/**
    Reads the last byte from the NFC bus.
*/
int i2c_ReadLast(void)
{
    I2CONCLR = AA;
    wait_for_SI();
    if (I2STAT != 0x58) {
      return -1;
    } else {
      return I2DAT;
    }
}

/**
    Sends the STOP condition.
*/
void i2c_Stop(void)
{	
    I2CONSET = 0x40;	
}
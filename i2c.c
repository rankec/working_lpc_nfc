//*************************************************
//** Polling I2C-Master Library
//** Created by Mike Schaub
//** Modified by Patrick Mania <pmania_at_tzi.de>
//** Version: 1.0
//** Description:
//** This Library provides I2C-Master Functions on LPC2000 Microcontrollers.
//** The Code was tested on a LPC2124 and LPC2194 
//** 
//** Compiled with WinARM
//*************************************************

#include <targets/LPC2000.h>
#include "i2c.h"

// Initialize I2C
void i2c_Init(){
        PINSEL0 |= 0x50; 
	I2SCLL = I2SCLH = 400;	//	75kBits @ 58,98 <MHz
	I2CONSET = 0x40;		//	enable I2C bus interface
}

// Handle timeouts and interrupt action
static void wait_for_SI(void){
	unsigned long timeout = 1600000;

	I2CONCLR = 8;								//	clear SI starts action
	while (timeout-- && !(I2CONSET & 8));		//	check SI with timeout
}


//Send "START" condition
int i2c_Start(int addr){
	I2CONCLR = 0x14;								//	clear STO, AA
	I2CONSET = 0x28;								//	set STA, SI
	wait_for_SI();
	I2CONCLR = 0x20;								//	clear STA
	if (I2STAT > 0x10) return(-1);				//	bus is busy
	I2DAT = addr;									//	set address
	wait_for_SI();
	return (I2STAT != 0x40 && I2STAT != 0x18);	//  no acknowledge
}

//Write on I2C bus
int i2c_Write(unsigned char *buf, unsigned count){
	while (count--)
	{
		I2DAT = *buf++;						//	load data into I2DAT-Register
		wait_for_SI();
		if (I2STAT != 0x28) return 1;		//	no acknowledge
	}
	return 0;
}

//Read from I2C bus
int i2c_Read(){
  I2CONSET=AA; // Assert Acknowledge
  wait_for_SI();
  if(I2STAT!=0x50 && I2STAT!=0x40){
    return -1; // Not ok
  }else{
    return I2DAT;
  }
}

//Read last byte from I2C Bus
int i2c_ReadLast(){
  I2CONCLR=AA; // Assert Acknowledge
  wait_for_SI();
  if(I2STAT!=0x58){
    return -1; // Not ok
  }else{
    return I2DAT;
  }
}

//Send "STOP" condition
void i2c_Stop(void){	
	I2CONSET = 0x40;		//	Re-enable I2C bus interface
}
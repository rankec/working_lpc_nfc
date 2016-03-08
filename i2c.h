#ifndef _I2C_H
#define _I2C_H

#define STA	0x20
#define SIC	0x08
#define SI	0x08
#define STO	0x10
#define STAC 0x20
#define AA	0x04

#define R 1// Read Bit +1

// STOP Makro
#define STOPI2C		I2C_I2CONCLR = SIC; I2C_I2CONSET=STO; while((I2C_I2CONSET&STO));

void i2c_init();

static void wait_for_SI(void);

int i2c_Start(int addr);

int i2c_Write(unsigned char *buf, unsigned count);

int i2c_Read();

int i2c_ReadLast();

void i2c_Stop(void);

#endif

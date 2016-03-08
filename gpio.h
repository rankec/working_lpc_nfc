/*
    The GPIO driver.
*/

#ifndef _GPIO_H
#define _GPIO_H


#include <targets/LPC2000.h>


void GPIO_InitPin(int high, int pin);

int GPIO_GetPinValue(int high, int pin);

void GPIO_SetPinValue(int high, int status, int pin);


#endif

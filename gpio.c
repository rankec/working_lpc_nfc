/*
    The GPIO driver.
*/


#include "gpio.h"


/**
    Initializes a pin.
    This function sets the corresponding bit in the IODIR register.
    @param high Whether to target a pin starting with 0 or with 1.
    @param pin The number of the pin to be initialized.
*/
void GPIO_InitPin(int high, int pin)
{
    *(high ? &IO1DIR : &IO0DIR) |= (1 << pin);
}


/**
    Returns the value of a pin.
    The pin needs to be initialized using GPIO_InitPin first.
    @param high Whether to target a pin starting with 0 or with 1.
    @param pin The number of the pin to be read.
    @return The value of the pin (0 or 1).
    @see GPIO_InitPin
*/
int GPIO_GetPinValue(int high, int pin)
{
    return *(high ? &IO1PIN : &IO0PIN) & (1 << pin);
}


/**
    Sets the value of a pin.
    The pin needs to be initialized using GPIO_InitPin first.
    @param high Whether to target a pin starting with 0 or with 1.
    @param status The value to set the pin to (0 or 1).
    @param pin The number of the pin to be set.
    @see GPIO_InitPin
*/
void GPIO_SetPinValue(int high, int status, int pin)
{
    volatile unsigned long *reg = (high
    ? (status ? &IO1CLR : &IO1SET)
    : (status ? &IO0CLR : &IO0SET));

    *reg |= (1 << pin);
}
/*
    The LCD driver.
*/

#ifndef _HD44780U_H
#define _HD44780U_H


#include <targets/LPC2000.h>


void LCD_Init(void);

void LCD_Cmd(unsigned int cmd);

void LCD_Enable(void);

void LCD_delay(void);

void LCD_WriteChar(char c);

void LCD_WriteString(char *string);

void LCD_Clear(void);

void LCD_Linebreak(void);

void LCD_AppendCharToScreen(char c);

void LCD_ClearBuffer(void);


#endif

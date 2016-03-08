/*
    The LCD driver.
*/


#include "HD44780U.h"


char s[32] = ""; /**< An internal buffer. */


/**
    Initializes the LCD.
    This function must be called before the LCD can be used.
*/
void LCD_Init(void)
{
    // Alle Werte haben einen Offset von 8 um auf Port 8-15 statt 0-7 zu laufen
    IO0DIR |= 0xFF00; // P0.8-P0.15 sind der Data Output (8 Bit mode)
    IO1DIR |= (1 << 16) | (1 << 17); // P1.16 und P1.17 sind Control Pins

    // LCD Init Sequenz
    LCD_delay();
    LCD_Cmd(0x3C00); // Setze das "Function Set": 8 Bit Mode, 2 Zeilen, 5x10
    LCD_Cmd(0x0F00); // Setze den "Display Switch": Display an, Cursor an, Blinken an
}


/**
    Sends a command to the LCD.
    @param The command to be sent.
*/
void LCD_Cmd(unsigned int cmd)
{
    IO0PIN = cmd; // Stelle den Instruction/Command Code bereit
    LCD_Enable(); // Schalte kurz den "E" Pin an, um die Instruction zu verarbeiten
}


/**
    Enables the LCD.
*/
void LCD_Enable(void)
{
    LCD_delay();
    IO1PIN |= (1 << 17);
    LCD_delay();
    IO1PIN &= ~(1 << 17);
    LCD_delay();
}


/**
    Pauses execution for a bit.
    This function is called to delay the program execution to give the LCD time to... do things.
*/
void LCD_delay(void)
{
    int x = 0;
    for (int i = 0; i < 19999; i++) {
        x++;
    }
}


/**
    Displays a single character on the screen.
    @param c The character to be displayed.
*/
void LCD_WriteChar(char c)
{
    IO1PIN |= (1 << 16); // Wechsele zum "Data Mode"
    IO0PIN = ((int) c << 8); // Character Code übertragen, aber um 8 Pins verschoben
    LCD_Enable(); // Schalte kurz den "E" Pin an, um die Instruction zu verarbeiten
}


/**
    Displays an entire string on the screen.
    This function automatically inserts line breaks and page breaks. It will wait a bit before switching to a new page, giving you time to read the text.
    @param string The string to be displayed.
*/
void LCD_WriteString(char *string)
{
    int characterCount = 0;
    int characterNewPageDelay = 100;
    
    LCD_Clear();

    int c = 0;
    while (string[c] != '\0') {

        characterCount++;
        
        if (characterCount == 17) {
            LCD_Linebreak();
        }
        if (characterCount == 33) {
            for (int i = 0; i < characterNewPageDelay; i++) {
                LCD_delay();
            }
            LCD_Clear();
            characterCount = 0;
        }
        
        LCD_WriteChar(string[c]); 

        c++;
    }
    characterCount = 0;

}


/**
    Clears the screen.
*/
void LCD_Clear(void)
{
    IO1CLR |= (1 << 16);
    LCD_Cmd(0x0600); // Setze das "Input Set": Increment Mode
    LCD_Cmd(0x0100); // Veranlasse einen Reset: Screen clear, Cursor auf Standard
    LCD_Cmd(0x8000); // Wird beim ersten LCD Init nicht benötigt, würde aber den Cursor nochmal zurücksetzen.
}


/**
    Inserts a line break on the screen.
*/
void LCD_Linebreak(void)
{
    IO1CLR |= (1 << 16);
    LCD_Cmd (0x8000 + 0x4000); // Gehe in die zweite Zeile
}


/**
    Appends a character to the screen.
    This function does not clear the contents of the screen; it just adds a character to them.
    @param c The character to be appended.
*/
void LCD_AppendCharToScreen(char c)
{
    int len = strlen(s);
    s[len] = c;
    s[len+1] = '\0';
    LCD_WriteString(s);
}


/**
    Clears the internal buffer.
*/
void LCD_ClearBuffer(void)
{
    s[0] = '\0';
}
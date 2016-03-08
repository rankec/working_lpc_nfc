#include "stdio.h"
#include <libarm.h>
#include <targets/LPC2000.h>

#include <keypad.h>
#include <HD44780U.h>
#include <PN532.h>
#include <i2c.h>


int main(void) {
        
        LCD_Init();
        LCD_Clear();
        Keypad_Init();
        i2c_Init();
        PN532_init();
        

        char c;
        int writeMode = 0;

        char buffer[32] = "";


        while (1)
        {
            c = Keypad_WaitAndGetKey();
            
            if(c == '*')
            {
                LCD_ClearBuffer();
                writeMode = 1 - writeMode;
                buffer[0] = '\0';
                LCD_WriteString(writeMode ? "Write mode..." : "Write ended.");
            }

            //read from NFC
            else if(c == '#' && writeMode == 0)
            {
              
              LCD_WriteString("Read from NFC...");
              
              if(readStringFromNFC(buffer))
              {
                   LCD_WriteString(buffer); 
              }
              else
              {
               LCD_WriteString("No Data"); 
              }
              
            }
            //write to NFC
            else if(writeMode)
            {
              LCD_AppendCharToScreen(c);

              int len = strlen(buffer);
              buffer[len] = c;
              buffer[len+1] = '\0';
              if(!writeStringToNFC(buffer))
              {
                debug_printf("%s\n", buffer);
                LCD_WriteString("Error 63."); 
              }
            }
            else{
              LCD_AppendCharToScreen(c);
            }

            
        }
}

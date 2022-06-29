#ifndef __SHIFT_LCD_DISPLAY_H__
#define __SHIFT_LCD_DISPLAY_H__
#include "pico/stdlib.h"
#include "ShiftRegister.h"



void init_lcd(S595OutPins* psPins);
void put_clear(S595OutPins* psPins);
void put_lines(S595OutPins* psPins, char* szLine1, char* szLine2);
void put_to_address(S595OutPins* psPins, char* sz, uint uiAddress);


#endif // __SHIFT_LCD_DISPLAY_H__


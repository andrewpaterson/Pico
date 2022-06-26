#ifndef __LCD_DISPLAY_H__
#define __LCD_DISPLAY_H__
#include "pico/stdlib.h"


void init_lcd(void);
void put_clear(void);
void put_lines(char* szLine1, char* szLine2);

void sleep_us_high_power(uint delay);


#endif // __LCD_DISPLAY_H__


#ifndef __GENERAL_PINS_H__
#define __GENERAL_PINS_H__
#include "pico/stdlib.h"


#define PIN_NOT_SET     (-1)


void sleep_us_high_power(uint delay);
int make_8bit_mask(int* paiPins, int iValue);
int make_4bit_mask(int* paiPins, int iValue);


#endif // __GENERAL_PINS_H__


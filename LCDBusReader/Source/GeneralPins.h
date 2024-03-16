#ifndef __GENERAL_PINS_H__
#define __GENERAL_PINS_H__
#include "pico/stdlib.h"


#define PIN_NOT_SET     (-1)


uint32_t make_8bit_mask(int* paiPins, uint32_t iValue);
uint32_t make_4bit_mask(int* paiPins, uint32_t iValue);


#endif // __GENERAL_PINS_H__


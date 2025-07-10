#ifndef __GENERAL_PINS_H__
#define __GENERAL_PINS_H__
#include "pico/stdlib.h"
#include "PrimitiveTypes.h"


#define PIN_NOT_SET     (-1)


uint32 make_8bit_mask(uint32* paiPins, uint32 iValue);
uint32 make_4bit_mask(uint32* paiPins, uint32 iValue);


#endif // __GENERAL_PINS_H__


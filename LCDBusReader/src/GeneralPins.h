#ifndef __GENERAL_PINS_H__
#define __GENERAL_PINS_H__
#include "pico/stdlib.h"


#define PIN_NOT_SET     (-1)


struct SDualHexDigitPins
{
    uint uiEnableDigit0;
    uint uiEnableDigit1;
    uint auiDigitsGPIO[8];

    void Init(  uint uiEnableDigit0,
                uint uiEnableDigit1,
                uint uiDigit1Bit3GPIO,
                uint uiDigit1Bit2GPIO,
                uint uiDigit1Bit1GPIO,
                uint uiDigit1Bit0GPIO,
                uint uiDigit0Bit3GPIO,
                uint uiDigit0Bit2GPIO,
                uint uiDigit0Bit1GPIO,
                uint uiDigit0Bit0GPIO);
};


void sleep_us_high_power(uint delay);

void init_dual_hex(SDualHexDigitPins* psPins);
void set_dual_hex_value(SDualHexDigitPins* psPins, int iValue);


#endif // __GENERAL_PINS_H__


#ifndef __HEX_DISPLAY_H__
#define __HEX_DISPLAY_H__
#include "GeneralPins.h"


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


void init_dual_hex(SDualHexDigitPins* psPins);
void set_dual_hex_value(SDualHexDigitPins* psPins, int iValue);


#endif // __HEX_DISPLAY_H__


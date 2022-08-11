#include "HEXDisplay.h"


void SDualHexDigitPins::Init(uint uiEnableDigit0,
                             uint uiEnableDigit1,
                             uint uiDigit1Bit3GPIO,
                             uint uiDigit1Bit2GPIO,
                             uint uiDigit1Bit1GPIO,
                             uint uiDigit1Bit0GPIO,
                             uint uiDigit0Bit3GPIO,
                             uint uiDigit0Bit2GPIO,
                             uint uiDigit0Bit1GPIO,
                             uint uiDigit0Bit0GPIO)
{
    this->uiEnableDigit0 = uiEnableDigit0;
    this->uiEnableDigit1 = uiEnableDigit1;

    auiDigitsGPIO[0] = uiDigit0Bit0GPIO;
    auiDigitsGPIO[1] = uiDigit0Bit1GPIO;
    auiDigitsGPIO[2] = uiDigit0Bit2GPIO;
    auiDigitsGPIO[3] = uiDigit0Bit3GPIO;

    auiDigitsGPIO[4] = uiDigit1Bit0GPIO;
    auiDigitsGPIO[5] = uiDigit1Bit1GPIO;
    auiDigitsGPIO[6] = uiDigit1Bit2GPIO;
    auiDigitsGPIO[7] = uiDigit1Bit3GPIO;
}


int make_dual_hex_digit_mask(SDualHexDigitPins* psPins, int iDigits)
{
    int iMask = (iDigits & 0x01 ? (1 << psPins->auiDigitsGPIO[0]) : 0) | 
                (iDigits & 0x02 ? (1 << psPins->auiDigitsGPIO[1]) : 0) | 
                (iDigits & 0x04 ? (1 << psPins->auiDigitsGPIO[2]) : 0) | 
                (iDigits & 0x08 ? (1 << psPins->auiDigitsGPIO[3]) : 0) | 
                (iDigits & 0x10 ? (1 << psPins->auiDigitsGPIO[4]) : 0) |
                (iDigits & 0x20 ? (1 << psPins->auiDigitsGPIO[5]) : 0) | 
                (iDigits & 0x40 ? (1 << psPins->auiDigitsGPIO[6]) : 0) | 
                (iDigits & 0x80 ? (1 << psPins->auiDigitsGPIO[7]) : 0);

    return iMask;
}


void init_dual_hex(SDualHexDigitPins* psPins)
{
    int iMask;

    iMask = make_dual_hex_digit_mask(psPins, 0xff);

    gpio_init_mask(iMask);
    gpio_set_dir_out_masked(iMask);
}


void set_dual_hex_value(SDualHexDigitPins* psPins, int iValue)
{
    int iMask = make_dual_hex_digit_mask(psPins, 0xff);
    iValue = make_dual_hex_digit_mask(psPins, iValue);
    gpio_put_masked(iMask, iValue);
}


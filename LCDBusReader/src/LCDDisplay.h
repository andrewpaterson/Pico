#ifndef __LCD_DISPLAY_H__
#define __LCD_DISPLAY_H__
#include "pico/stdlib.h"


struct S11BitLCDPins
{
    uint uiRegisterSelectGPIO;
    uint uiReadWriteBGPIO;
    uint uiEnableGPIO;
    uint auiDataGPIO[8];

    void Init(  uint uiRegisterSelectGPIO,
                uint uiReadWriteBGPIO,
                uint uiEnableGPIO,
                uint uiData0GPIO,
                uint uiData1GPIO,
                uint uiData2GPIO,
                uint uiData3GPIO,
                uint uiData4GPIO,
                uint uiData5GPIO,
                uint uiData6GPIO,
                uint uiData7GPIO);
};


void init_lcd(S11BitLCDPins* psPins);
void put_clear(S11BitLCDPins* psPins);
void put_lines(S11BitLCDPins* psPins, char* szLine1, char* szLine2);


#endif // __LCD_DISPLAY_H__


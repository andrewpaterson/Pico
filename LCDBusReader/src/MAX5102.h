#ifndef __MAX5102_H__
#define __MAX5102_H__
#include "GeneralPins.h"


struct SMAX5102Pins
{
    int     iA0Pin;
    int     iWRBPin;
    int     aiDataPins[8];

    void Init(int iA0Pin, int iWRBPin, int iData0, int iData1, int iData2, int iData3, int iData4, int iData5, int iData6, int iData7);
};


void init_max5102(SMAX5102Pins* psPins);
int make_max5102_mask(SMAX5102Pins* psPins, bool bA0, bool bWRB, int iValue);


#endif // __MAX5102_H__


#include "MAX5102.h"


void SMAX5102Pins::Init(int iA0Pin, int iWRBPin, int iData0, int iData1, int iData2, int iData3, int iData4, int iData5, int iData6, int iData7)
{
    this->iA0Pin = iA0Pin;
    this->iWRBPin = iWRBPin;
    this->aiDataPins[0] = iData0;
    this->aiDataPins[1] = iData1;
    this->aiDataPins[2] = iData2;
    this->aiDataPins[3] = iData3;
    this->aiDataPins[4] = iData4;
    this->aiDataPins[5] = iData5;
    this->aiDataPins[6] = iData6;
    this->aiDataPins[7] = iData7;
}


void init_max5102(SMAX5102Pins* psPins)
{
    int iMask = make_max5102_mask(psPins, true, true, 0xff);
    
    gpio_init_mask(iMask);
    gpio_set_dir_out_masked(iMask);
}


int make_max5102_mask(SMAX5102Pins* psPins, bool bA0, bool bWRB, int iValue)
{
    int iDataMask = (iValue & 0x01 ? (1 << psPins->aiDataPins[0]) : 0) | 
                (iValue & 0x02 ? (1 << psPins->aiDataPins[1]) : 0) | 
                (iValue & 0x04 ? (1 << psPins->aiDataPins[2]) : 0) | 
                (iValue & 0x08 ? (1 << psPins->aiDataPins[3]) : 0) | 
                (iValue & 0x10 ? (1 << psPins->aiDataPins[4]) : 0) |
                (iValue & 0x20 ? (1 << psPins->aiDataPins[5]) : 0) | 
                (iValue & 0x40 ? (1 << psPins->aiDataPins[6]) : 0) | 
                (iValue & 0x80 ? (1 << psPins->aiDataPins[7]) : 0);

    int iCommandMask = (bA0 ? (1 << psPins->iA0Pin) : 0) |
                        (bWRB ? (1 << psPins->iWRBPin) : 0);

    return iCommandMask | iDataMask;
}


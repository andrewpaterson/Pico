#include "GeneralPins.h"


uint32 make_8bit_mask(uint8* paiPins, uint8 iValue)
{
    uint32 uiDataMask = (iValue & 0x01 ? (1ul << paiPins[0]) : 0) | 
                        (iValue & 0x02 ? (1ul << paiPins[1]) : 0) | 
                        (iValue & 0x04 ? (1ul << paiPins[2]) : 0) | 
                        (iValue & 0x08 ? (1ul << paiPins[3]) : 0) | 
                        (iValue & 0x10 ? (1ul << paiPins[4]) : 0) |
                        (iValue & 0x20 ? (1ul << paiPins[5]) : 0) | 
                        (iValue & 0x40 ? (1ul << paiPins[6]) : 0) | 
                        (iValue & 0x80 ? (1ul << paiPins[7]) : 0);
    return uiDataMask;
}


uint32 make_4bit_mask(uint8* paiPins, uint8 iValue)
{
    uint32 uiDataMask = (iValue & 0x01 ? (1ul << paiPins[0]) : 0) | 
                        (iValue & 0x02 ? (1ul << paiPins[1]) : 0) | 
                        (iValue & 0x04 ? (1ul << paiPins[2]) : 0) | 
                        (iValue & 0x08 ? (1ul << paiPins[3]) : 0);
    return uiDataMask;
}


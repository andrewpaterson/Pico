#include "GeneralPins.h"


void sleep_us_high_power(uint delay)
{
    uint64_t start = time_us_64();
    uint64_t expectedEnd = start + delay;
    uint64_t end = start;

    while (expectedEnd > end)
    {
        end = time_us_64();
    }
}


int make_8bit_mask(int* paiPins, int iValue)
{
    int iDataMask = (iValue & 0x01 ? (1 << paiPins[0]) : 0) | 
                    (iValue & 0x02 ? (1 << paiPins[1]) : 0) | 
                    (iValue & 0x04 ? (1 << paiPins[2]) : 0) | 
                    (iValue & 0x08 ? (1 << paiPins[3]) : 0) | 
                    (iValue & 0x10 ? (1 << paiPins[4]) : 0) |
                    (iValue & 0x20 ? (1 << paiPins[5]) : 0) | 
                    (iValue & 0x40 ? (1 << paiPins[6]) : 0) | 
                    (iValue & 0x80 ? (1 << paiPins[7]) : 0);
    return iDataMask;
}


int make_4bit_mask(int* paiPins, int iValue)
{
    int iDataMask = (iValue & 0x01 ? (1 << paiPins[0]) : 0) | 
                    (iValue & 0x02 ? (1 << paiPins[1]) : 0) | 
                    (iValue & 0x04 ? (1 << paiPins[2]) : 0) | 
                    (iValue & 0x08 ? (1 << paiPins[3]) : 0);
    return iDataMask;
}


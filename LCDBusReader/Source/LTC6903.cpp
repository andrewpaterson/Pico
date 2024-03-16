#include <math.h>
#include "LTC6903.h"


int get_OCT(int iHertz)
{
    int iOct = 0;
    if (iHertz > 34'010'000)
    {
        iOct = 15;
    }
    else if (iHertz > 17'010'000)
    {
        iOct = 14;
    }
    else if (iHertz > 8'503'000)
    {
        iOct = 13;
    }
    else if (iHertz > 4'252'000)
    {
        iOct = 12;
    }
    else if (iHertz > 2'126'000)
    {
        iOct = 11;
    }
    else if (iHertz > 1'063'000)
    {
        iOct = 10;
    }
    else if (iHertz > 531'400)
    {
        iOct = 9;
    }
    else if (iHertz > 265'700)
    {
        iOct = 8;
    }
    else if (iHertz > 132'900)
    {
        iOct = 7;
    }
    else if (iHertz > 66'430)
    {
        iOct = 6;
    }
    else if (iHertz > 33'220)
    {
        iOct = 5;
    }
    else if (iHertz > 16'610)
    {
        iOct = 4;
    }
    else if (iHertz > 8304)
    {
        iOct = 3;
    }
    else if (iHertz > 4152)
    {
        iOct = 2;
    }
    else if (iHertz > 2076)
    {
        iOct = 1;
    }
   return iOct;
}


int get_DAC(int iOct, int iHertz)
{
    double fRight = pow(2.0, 10.0 + (double)iOct);
    double fDAC = 2048.0 - ((2078.0 * fRight) / (double)iHertz);
    fDAC = round(fDAC);
    return (int)fDAC;
}


void put_LTC6903_frequency(SSPIPins* psSPI, int iHertz)
{
    int iOct = get_OCT(iHertz);
    int iDac = get_DAC(iOct, iHertz);

    uint16_t uiData = (iOct << 12) | (iDac << 2) | 0;

    write_spi(psSPI, 0xFFFF);
    write_spi(psSPI, uiData);
}


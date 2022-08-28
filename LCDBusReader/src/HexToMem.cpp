#include "HexToMem.h"


uint8_t hex_to_byte(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    if (c >= 'A' && c <= 'F')
    {
        return c - 'A' + 10;
    }
    if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + 10;
    }

    return 0xFF;
}


int read_hex_string_into_memory(uint8_t* pvDest, int iDestLength, char* szSource)
{
    bool bHigh = true;
    int i;
    for (i = 0; i < (iDestLength * 2); i++)
    {
        char c = szSource[i];
        if (c == '\0')
        {
            //Return the number of nybbles read.
            return i;
        }

        uint8_t uiValue = hex_to_byte(c);
        if (uiValue == 0xFF)
        {
            return -1;
        }

        if (bHigh)
        {
            uiValue <<= 4;
            *pvDest = uiValue;
        }
        else
        {
            *pvDest |= uiValue;
            pvDest++;
        }
        bHigh = !bHigh;
    }
    return i;    
}


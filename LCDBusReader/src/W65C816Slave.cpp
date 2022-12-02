#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "W65C816Slave.h"


void CW65C816Slave::Init(SW65C816Pins* psPins)
{
    CW65C816Bus::Init(psPins);
    miData = 0;
    miAddress = 0;
}


bool CW65C816Slave::ExecuteMessage(char* szMessage, char* szResponse)
{
    if (memcmp(szMessage, "IO:Z", 4) == 0)
    {
        DisableIO();
        strcpy(szResponse, "OK\n");
        return true;
    }
    else if (memcmp(szMessage, "A:", 2) == 0)
    {
        miAddress = strtol(&szMessage[2], NULL, 16);
        if (miAddress >= 0 && miAddress <= 0xFFFF)
        {
            strcpy(szResponse, "OK\n");
            return true;
        }
        else
        {
            strcpy(szResponse, "A#\n");
            return false;
        }
    }
    else if (memcmp(szMessage, "D:", 2) == 0)
    {
        miData = strtol(&szMessage[2], NULL, 16);
        if (miData >= 0 && miData <= 0xFF)
        {
            strcpy(szResponse, "OK\n");
            return true;
        }
        else
        {
            strcpy(szResponse, "D#\n");
            return false;
        }
    }
    else if (memcmp(szMessage, "IO:A", 4) == 0)
    {
        AddressOutDataIn(miAddress);
        strcpy(szResponse, "OK\n");
        return true;
    }
    else if (memcmp(szMessage, "IO:D+A", 6) == 0)
    {
        AddressOutDataOut(miData, miAddress);
        strcpy(szResponse, "OK\n");
        return true;
    }
    else if (memcmp(szMessage, "READ", 4) == 0)
    {
        miData = Read();
        if (miData < 0x10)
        {
            szResponse[0] = '0';
            itoa(miData, &szResponse[1], 16);
        }
        else
        {
            itoa(miData, szResponse, 16);
        }
        szResponse[2] = '\n';
        szResponse[3] = '\0';
        return true;
    }
    else    
    {
        strcpy(szResponse, "B#");
        return false;
   }
}


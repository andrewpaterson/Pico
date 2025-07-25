#include "TesterPins.h"
#include "TesterState.h"
#include "Commands.h"


char    gszMessage[256];


size HexValue(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    else if (c >= 'A' && c <= 'F')
    {
        return c - 'A' + 10;
    }
    else
    {
        return SIZE_MAX;
    }
}


char HexChar(uint8 uiValue)
{
    if (uiValue <= 9)
    {
        return '0' + uiValue;
    }
    else if (uiValue <= 0xf)
    {
        return 'A' + uiValue - 10;
    }
    else
    {
        return '\0';
    }
}


size HexData(uint8* puiData, char* szCommand, size uiLength)
{
    size    ui;
    char    c;
    size    uiValue;
    size    uiSourceIndex;
    size    uiDestIndex;

    if (uiLength == 0)
    {
        strcpy(gszMessage, "Error: Expected data");
        return SIZE_MAX;
    }

    uiDestIndex = 0;
    for (ui = 0; ui < uiLength; ui++)
    {
        uiSourceIndex = uiLength - ui - 1;
        c = szCommand[uiSourceIndex];
        uiValue = HexValue(c);
        if (uiValue == SIZE_MAX)
        {
            strcpy(gszMessage, "Error: Expected ASCII Hex data");
            return SIZE_MAX;
        }
        
        if ((ui & 1) == 1)
        {
            puiData[uiDestIndex] |= (uint8)(uiValue << 4);
            uiDestIndex++;
        }
        else
        {
            puiData[uiDestIndex] = (uint8)uiValue;
        }
    }

    if (ui & 1 == 1)
    {
         uiDestIndex++;
    }
    return uiDestIndex;
}


void SetAllDataToZero(void)
{
    SetAllToData(0);
    PulseWriteData();
}


size ParseWithOffset(uint8* puiData, size* puiOffset, char* szCommand, size uiLength)
{
    char    c;
    size    uiBytes;
    size    ui;

    if (uiLength > 1)
    {
        c = szCommand[0];
        *puiOffset = HexValue(c);
        if (*puiOffset == SIZE_MAX)
        {
            strcpy(gszMessage, "Error: Expected offset");
            return SIZE_MAX;
        }

        uiBytes = HexData(puiData, &szCommand[1], uiLength - 1);
        return uiBytes;
    }
    else
    {
        strcpy(gszMessage, "Error: Expected offset");
        return SIZE_MAX;
    }
}


void SetOffsetData(char* szCommand, size uiLength)
{
    size    uiOffset;
    size    uiBytes;
    uint8   auiData[16];
    size    ui;

    uiBytes = ParseWithOffset(auiData, &uiOffset, szCommand, uiLength);
    if (uiBytes != SIZE_MAX)
    {
        for (ui = 0; ui< uiBytes; ui++)
        {
            WriteData(ADDRESS_WRITE_GPIO__0__7 - uiOffset - ui, auiData[ui]);
        }
        PulseWriteData();
    }
}


void SetData(char* szCommand, size uiLength)
{
    size    uiBytes;
    size    ui;
    uint8   auiData[16];

    memset(auiData, 0, 16);
    uiBytes = HexData(auiData, &szCommand[0], uiLength);
    if (uiBytes != SIZE_MAX)
    {
        for (ui = 0; ui< uiBytes; ui++)
        {
            WriteData(ADDRESS_WRITE_GPIO__0__7 - ui, auiData[ui]);
        }
        PulseWriteData();
    }
}


void SetAllOutputsToInput(void)
{
    SetAllToOutput(0);
    PulseWriteData();
}


void SetOffsetOutputs(char* szCommand, size uiLength)
{
    size    uiOffset;
    size    uiBytes;
    uint8   auiData[16];
    size    ui;

    uiBytes = ParseWithOffset(auiData, &uiOffset, szCommand, uiLength);
    if (uiBytes != SIZE_MAX)
    {
        for (ui = 0; ui< uiBytes; ui++)
        {
            WriteData(ADDRESS_OUTPUT_GPIO__0__7 - uiOffset - ui, auiData[ui]);
        }
        PulseWriteData();
    }
}


void SetOutputs(char* szCommand, size uiLength)
{
    size    uiBytes;
    size    ui;
    uint8   auiData[16];

    uiBytes = HexData(auiData, &szCommand[0], uiLength);
    if (uiBytes != SIZE_MAX)
    {
        for (ui = 0; ui< uiBytes; ui++)
        {
            WriteData(ADDRESS_OUTPUT_GPIO__0__7 - ui, auiData[ui]);
        }
        PulseWriteData();
    }
}


void GetOffsetData(uint8 uiStart, uint8 uiEndInclusive)
{
    size    ui;
    size    uiIndex;
    uint8   uiAddress;
    uint8   uiData;
    char    cHighNybble;
    char    cLowNybble;

    if (uiEndInclusive < uiStart)
    {
        strcpy(gszMessage, "Error: Start must be <= end");
        return;
    }

    PulseReadData();
    gszMessage[0] = 'R';
    uiIndex = (uiEndInclusive - uiStart) * 2 + 2;
    for (ui = uiStart; ui <= uiEndInclusive; ui++)
    {
        uiAddress = aiReadAddresses[ui];
        uiData = ReadData(uiAddress);
        uiData = ReverseByte(uiData);
        cLowNybble = HexChar(uiData & 0xF);
        cHighNybble = HexChar(uiData >> 4);
        gszMessage[uiIndex] = cLowNybble;
        uiIndex--;
        gszMessage[uiIndex] = cHighNybble;
        uiIndex--;
    }
    gszMessage[(uiEndInclusive - uiStart) * 2 + 3] = '\0';
}


void GetAllData(void)
{
    GetOffsetData(0x0, 0xf);
}


void GetData(char* szCommand, size uiLength)
{
    size    uiStart;
    size    uiEndInclusive;
    char    c;
    size    uiBytes;
    size    ui;

    if (uiLength == 3)
    {
        c = szCommand[0];
        uiStart = HexValue(c);
        if (uiStart == SIZE_MAX)
        {
            strcpy(gszMessage, "Error: Expected start offset");
            return;
        }
        c = szCommand[1];
        if (c != '_')
        {
            strcpy(gszMessage, "Error: Expected underscore");
            return;
        }
        c = szCommand[2];
        uiEndInclusive = HexValue(c);
        if (uiEndInclusive == SIZE_MAX)
        {
            strcpy(gszMessage, "Error: Expected end offset");
            return;
        }

        GetOffsetData(uiStart, uiEndInclusive);
    }
    else if (uiLength == 1)
    {
        c = szCommand[0];
        uiStart = HexValue(c);
        if (uiStart == SIZE_MAX)
        {
            strcpy(gszMessage, "Error: Expected start offset");
            return;
        }

        GetOffsetData(uiStart, uiStart);
    }
    else
    {
        strcpy(gszMessage, "Error: Expected offset one or two offsets");
    }
}


char* ExecuteCommand(char* szCommand, size uiLength)
{
    //* Wx..x - Write data bits e.g. Dff00ff00ff00ff00ff00ff00ff00ff00 writes 1s to bits 127 to 120 then 0s to bits 119 to 112 etc... 
    //                            Dff writes 1s to bits 7 to 0.
    //* W_ox..x - Write data bits with byte offset e.g. D_1ff00 writes 1s to bits 23 to 16 then 0s to bits 15 to 8.
    //                                               D_0f writes 1s to bits 3 to 0 (same as Df)
    //* W - Write all data bits with 0.
    //* Ox..x - Set GPIO as outputs from bits e.g. Off00ff00ff00ff00ff00ff00ff00ff00 sets bits 127 to 120 to outputs then sets bits 119 to 112 to inputs etc... 
    //* O_ox..x - same as Wo_xxx but sets outputs.
    //* O - Set all GPIO as inputs
    //* Ro_p - Latch ALL input bits.  Return values from byte o to byte p inclusive.  GPIO set as output are also returned and should be masked off.
    //       e.g. R0_7 return all 128 bits of GPIO
    //* Ro - Same as Ro_p but for a single byte.
    //* R - Latch ALL input bits.  Return all values from byte 0 to 15 inclusive.
    
    //Note: you should call W before calling O to ensure the correct values are present when the GPIO is set to output.
    //Note: the first byte returned is the high byte.  The first nybble returned is the high nybble.
    
    gszMessage[0] = '\0';
    if (uiLength == 0)
    {
        strcpy(gszMessage, "Error: No command");
    }
    else if (uiLength == 1)
    {
        if (szCommand[0] == 'W')
        {
            SetAllDataToZero();
        }
        else if (szCommand[0] == 'O')
        {
            SetAllOutputsToInput();
        }
        else if (szCommand[0] == 'R')
        {
            GetAllData();
        }
        else
        {
            strcpy(gszMessage, "Error: Unknown command");
        }
    }
    else
    {
        if (MemCmp("W_", 2, szCommand, 2) == 0)
        {
            SetOffsetData(&szCommand[2], uiLength - 2);
        }
        else if (MemCmp("O_", 2, szCommand, 2) == 0)
        {
            SetOffsetOutputs(&szCommand[2], uiLength - 2);
        }
        else if (szCommand[0] == 'W')
        {
            SetData(&szCommand[1], uiLength - 1);
        }
        else if (szCommand[0] == 'O')
        {
            SetOutputs(&szCommand[1], uiLength - 1);
        }
        else if (szCommand[0] == 'R')
        {
            GetData(&szCommand[1], uiLength - 1);
        }
        else
        {
            strcpy(gszMessage, "Error: Unknown command");
        }
    }

    return gszMessage;
}


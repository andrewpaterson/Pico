#include "TesterPins.h"
#include "TesterState.h"
#include "Commands.h"


char    gszMessage[256];


void UseASCIIHex(void)
{

}


void Use8BitBytes(void)
{

}


void SetAllDataToZero(void)
{
    
}


void SetOffsetData(char* szCommand, size uiLength)
{
    
}


void SetData(char* szCommand, size uiLength)
{
    
}


void SetAllOutputsToInput(void)
{

}


void SetOffsetOutputs(char* szCommand, size uiLength)
{
    
}


void SetOutputs(char* szCommand, size uiLength)
{
    
}


void GetAllData(void)
{
    
}


void GetData(char* szCommand, size uiLength)
{
    
}


char* ExecuteCommand(char* szCommand, size uiLength)
{
    //* X - Use ASCII hex
    //* XX - Use 8bit bytes
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
    //Note: Offsets o and p are always ASCII hex even if mode is XX
    
    gszMessage[0] = '\0';
    if (uiLength == 0)
    {
        strcpy(gszMessage, "Error: No command");
    }
    else if (uiLength == 1)
    {
        if (szCommand[0] == 'X')
        {
            UseASCIIHex();
        }
        else if (szCommand[0] == 'W')
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
    }
    else
    {
        if (MemCmp("XX", 2, szCommand, uiLength) == 0)
        {
            Use8BitBytes();
        }
        else if (MemCmp("W_", 2, szCommand, 2) == 0)
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
    }

    return gszMessage;
}


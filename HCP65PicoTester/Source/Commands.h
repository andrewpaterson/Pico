#ifndef __COMMANDS_H__
#define __COMMANDS_H__
#include "PrimitiveTypes.h"


char    HexChar(uint8 uiValue);
size    HexValue(char c);

char*   ExecuteCommand(char* pszCommand, size uiLength);

void    SetAllDataToZero(void);
void    SetAllOutputToInput(void);
void    GetAllData(void);


#endif // __COMMANDS_H__


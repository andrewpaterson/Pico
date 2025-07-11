#ifndef __COMMANDS_H__
#define __COMMANDS_H__
#include "PrimitiveTypes.h"


void    SetAllDataToZero(void);
void    SetAllOutputToInput(void);
void    GetAllData(void);
char*   ExecuteCommand(char* pszCommand, size uiLength);


#endif // __COMMANDS_H__


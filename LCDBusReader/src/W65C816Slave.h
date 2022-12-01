#ifndef __W65C816_SLAVE_H__
#define __W65C816_SLAVE_H__
#include "W65C816Pins.h"


class CW65C816Slave
{
protected:
    int             miData;
    int             miAddress;
    SW65C816Pins*   mpsPins;

public:
    void    Init(SW65C816Pins* psPins);
    bool    ExecuteMessage(char* szMessage, char* szResponse);
};


#endif // __W65C816_SLAVE_H__


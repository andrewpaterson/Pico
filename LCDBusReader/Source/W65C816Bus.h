#ifndef __W65C816_BUS_H__
#define __W65C816_BUS_H__
#include "W65C816Pins.h"


class CW65C816Bus
{
protected:
    SW65C816Pins* mpsPins;

public:
    void Init(SW65C816Pins* psPins);

    void DisableIO(void);
    void AddressOutDataOut(uint uiData, uint uiAddress);
    void AddressOutDataIn(uint uiAddress);
    int  Read(void);
};


#endif // __W65C816_BUS_H__


#ifndef __W65C816_BUS_H__
#define __W65C816_BUS_H__
#include "W65C816Pins.h"

void w65_init(SW65C816Pins* psPins);
void w65_disable_io(SW65C816Pins* psPins);
void w65_write(SW65C816Pins* psPins, uint uiData, uint uiAddress);


#endif // __W65C816_BUS_H__


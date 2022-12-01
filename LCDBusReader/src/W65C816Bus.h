#ifndef __W65C816_BUS_H__
#define __W65C816_BUS_H__
#include "W65C816Pins.h"


void w65_init(SW65C816Pins* psPins);
void w65_disable_io(SW65C816Pins* psPins);
void w65_address_out_data_out(SW65C816Pins* psPins, uint uiData, uint uiAddress);
void w65_address_out_data_in(SW65C816Pins* psPins, uint uiAddress);
int  w65_read(SW65C816Pins* psPins);


#endif // __W65C816_BUS_H__


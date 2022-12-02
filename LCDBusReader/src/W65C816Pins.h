#ifndef __W65C816_PINS_H__
#define __W65C816_PINS_H__
#include "GeneralPins.h"


struct SW65C816Pins
{
    uint auiAddressGPIO[19];
    uint auiDataGPIO[8];
    uint uiPinMask;
    uint uiDataMask;
    uint uiAddressMask;

    void Init(  uint uiData0GPIO,
                uint uiData1GPIO,
                uint uiData2GPIO,
                uint uiData3GPIO,
                uint uiData4GPIO,
                uint uiData5GPIO,
                uint uiData6GPIO,
                uint uiData7GPIO,
                uint uiAddress0GPIO,
                uint uiAddress1GPIO,
                uint uiAddress2GPIO,
                uint uiAddress3GPIO,
                uint uiAddress4GPIO,
                uint uiAddress5GPIO,
                uint uiAddress6GPIO,
                uint uiAddress7GPIO,
                uint uiAddress8GPIO,
                uint uiAddress9GPIO,
                uint uiAddress10GPIO,
                uint uiAddress11GPIO,
                uint uiAddress12GPIO,
                uint uiAddress13GPIO,
                uint uiAddress14GPIO,
                uint uiAddress15GPIO,
                uint uiAddress16GPIO,
                uint uiAddress17GPIO,
                uint uiAddress18GPIO);
};


uint make_w65c816_address_mask(SW65C816Pins* psPins, uint uiAddress);
uint make_w65c816_data_mask(SW65C816Pins* psPins, uint uiData);
uint make_w65c816_pins_mask(SW65C816Pins* psPins, uint uiAddress, uint uiData);
uint mask_w65c816_to_data(SW65C816Pins* psPins, uint uiData);


#endif // __W65C816_PINS_H__


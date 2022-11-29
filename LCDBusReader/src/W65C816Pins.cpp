#include "W65C816Pins.h"


void SW65C816Pins::Init(uint uiData0GPIO,
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
                        uint uiAddress18GPIO)
{
    auiAddressGPIO[0] = uiAddress0GPIO;
    auiAddressGPIO[1] = uiAddress1GPIO;
    auiAddressGPIO[2] = uiAddress2GPIO;
    auiAddressGPIO[3] = uiAddress3GPIO;
    auiAddressGPIO[4] = uiAddress4GPIO;
    auiAddressGPIO[5] = uiAddress5GPIO;
    auiAddressGPIO[6] = uiAddress6GPIO;
    auiAddressGPIO[7] = uiAddress7GPIO;
    auiAddressGPIO[8] = uiAddress8GPIO;
    auiAddressGPIO[9] = uiAddress9GPIO;
    auiAddressGPIO[10] = uiAddress10GPIO;
    auiAddressGPIO[11] = uiAddress11GPIO;
    auiAddressGPIO[12] = uiAddress12GPIO;
    auiAddressGPIO[13] = uiAddress13GPIO;
    auiAddressGPIO[14] = uiAddress14GPIO;
    auiAddressGPIO[15] = uiAddress15GPIO;
    auiAddressGPIO[16] = uiAddress16GPIO;
    auiAddressGPIO[17] = uiAddress17GPIO;
    auiAddressGPIO[18] = uiAddress18GPIO;

    auiDataGPIO[0] = uiData0GPIO;
    auiDataGPIO[1] = uiData1GPIO;
    auiDataGPIO[2] = uiData2GPIO;
    auiDataGPIO[3] = uiData3GPIO;
    auiDataGPIO[4] = uiData4GPIO;
    auiDataGPIO[5] = uiData5GPIO;
    auiDataGPIO[6] = uiData6GPIO;
    auiDataGPIO[7] = uiData7GPIO;
}


uint mask_address_pin_value(SW65C816Pins* psPins, uint uiAddressBitIndex, uint uiValue)
{
    uint uiPin = psPins->auiAddressGPIO[uiAddressBitIndex];
    if (uiPin != PIN_NOT_SET)
    {
        uint uiPinBit = 1 << uiPin;
        uint uiAddressBit = 1 << uiAddressBitIndex;
        bool bValue = (uiValue & uiAddressBit) == uiAddressBit;
        if (bValue)
        {
            return uiPinBit;
        }
    }
    return 0;
}


uint mask_data_pin_value(SW65C816Pins* psPins, uint uiDataBitIndex, uint uiValue)
{
    uint uiPin = psPins->auiDataGPIO[uiDataBitIndex];
    if (uiPin != PIN_NOT_SET)
    {
        uint uiPinBit = 1 << uiPin;
        uint uiDataBit = 1 << uiDataBitIndex;
        bool bValue = (uiValue & uiDataBit) == uiDataBit;
        if (bValue)
        {
            return uiPinBit;
        }
    }
    return 0;
}


uint make_w65c816_pins_mask(SW65C816Pins* psPins, uint uiAddress, uint uiData)
{
    uint a0 = mask_address_pin_value(psPins, 0, uiAddress);
    uint a1 = mask_address_pin_value(psPins, 1, uiAddress);
    uint a2 = mask_address_pin_value(psPins, 2, uiAddress);
    uint a3 = mask_address_pin_value(psPins, 3, uiAddress);
    uint a4 = mask_address_pin_value(psPins, 4, uiAddress);
    uint a5 = mask_address_pin_value(psPins, 5, uiAddress);
    uint a6 = mask_address_pin_value(psPins, 6, uiAddress);
    uint a7 = mask_address_pin_value(psPins, 7, uiAddress);
    uint a8 = mask_address_pin_value(psPins, 8, uiAddress);
    uint a9 = mask_address_pin_value(psPins, 9, uiAddress);
    uint a10 = mask_address_pin_value(psPins, 10, uiAddress);
    uint a11 = mask_address_pin_value(psPins, 11, uiAddress);
    uint a12 = mask_address_pin_value(psPins, 12, uiAddress);
    uint a13 = mask_address_pin_value(psPins, 13, uiAddress);
    uint a14 = mask_address_pin_value(psPins, 14, uiAddress);
    uint a15 = mask_address_pin_value(psPins, 15, uiAddress);
    uint a16 = mask_address_pin_value(psPins, 16, uiAddress);
    uint a17 = mask_address_pin_value(psPins, 17, uiAddress);
    uint a18 = mask_address_pin_value(psPins, 18, uiAddress);

    uint uiAddressMask =  a0 |  a1 |  a2 |  a3 |  a4 |  a5 |  a6 |  a7 |
                          a8 |  a9 | a10 | a11 | a12 | a13 | a14 | a15 |
                         a16 | a17 | a18;

    uint d0 = mask_data_pin_value(psPins, 0, uiData);
    uint d1 = mask_data_pin_value(psPins, 1, uiData);
    uint d2 = mask_data_pin_value(psPins, 2, uiData);
    uint d3 = mask_data_pin_value(psPins, 3, uiData);
    uint d4 = mask_data_pin_value(psPins, 4, uiData);
    uint d5 = mask_data_pin_value(psPins, 5, uiData);
    uint d6 = mask_data_pin_value(psPins, 6, uiData);
    uint d7 = mask_data_pin_value(psPins, 7, uiData);

    uint uiDataMask =  d0 | d1 | d2 | d3 | d4 | d5 | d6 | d7;

    return uiAddressMask;
}


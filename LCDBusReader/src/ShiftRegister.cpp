#include "GeneralPins.h"
#include "ShiftRegister.h"


void S595OutPins::Init(uint uiOutputEnableBPin,
            uint uiShiftPin,
            uint uiStorageLatchPin,
            uint uiDataInPin,
            uint uiMasterResetPin)
{
    this->uiOutputEnableBPin = uiOutputEnableBPin;
    this->uiShiftPin = uiShiftPin;
    this->uiStorageLatchPin = uiStorageLatchPin;
    this->uiDataInPin = uiDataInPin;
    this->uiMasterResetPin = uiMasterResetPin;
}


void init_shift(S595OutPins* psPins)
{
    //psPins->uiOutputEnableBPin;
    //this->uiMasterResetPin;

    gpio_init(psPins->uiShiftPin);
    gpio_set_dir(psPins->uiShiftPin, true);

    gpio_init(psPins->uiStorageLatchPin);
    gpio_set_dir(psPins->uiStorageLatchPin, true);

    gpio_init(psPins->uiDataInPin);
    gpio_set_dir(psPins->uiDataInPin, true);
}


void shift_out(S595OutPins* psPins, uint16_t uiData)
{
    int         i;
    bool        bBit;

    for (i = 0; i < 16; i++)
    {
        bBit = (uiData & 0x8000) == 0x8000;
        uiData = uiData << 1;
  //      sleep_us_high_power(1);
        gpio_put(psPins->uiShiftPin, false);
        gpio_put(psPins->uiDataInPin, bBit);
  //      sleep_us_high_power(1);
        gpio_put(psPins->uiShiftPin, true);
    }
    gpio_put(psPins->uiShiftPin, false);

    gpio_put(psPins->uiStorageLatchPin, false);
//    sleep_us_high_power(1);
    gpio_put(psPins->uiStorageLatchPin, true);
  //  sleep_us_high_power(1);
    gpio_put(psPins->uiStorageLatchPin, false);
}


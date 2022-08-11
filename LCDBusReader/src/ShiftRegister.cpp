#include "GeneralPins.h"
#include "ShiftRegister.h"


void S595OutPins::Init(uint uiOutputEnableBPin,
            uint uiShiftPin,
            uint uiStorageLatchPin,
            uint uiDataOutPin,
            uint uiMasterResetPin)
{
    this->uiOutputEnableBPin = uiOutputEnableBPin;
    this->uiShiftPin = uiShiftPin;
    this->uiStorageLatchPin = uiStorageLatchPin;
    this->uiDataOutPin = uiDataOutPin;
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

    gpio_init(psPins->uiDataOutPin);
    gpio_set_dir(psPins->uiDataOutPin, true);
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
        gpio_put(psPins->uiDataOutPin, bBit);
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

void S165InPins::Init(  uint uiLatchPin, 
                        uint uiClockPin,
                        uint uiDataInPin,
                        bool bLatchHigh,
                        bool bDataInverted,
                        bool bDataOnClockLow)
{
    this->uiLatchPin = uiLatchPin;
    this->uiClockPin = uiClockPin;
    this->uiDataInPin = uiDataInPin;

    this->bLatchHigh = bLatchHigh;    
    this->bDataInverted = bDataInverted;
    this->bDataOnClockLow = bDataOnClockLow;
}
            
void init_shift(S165InPins* psPins);
uint shift_in(S165InPins* psPins);

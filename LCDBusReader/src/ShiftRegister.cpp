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
    gpio_set_dir(psPins->uiShiftPin, GPIO_OUT);

    gpio_init(psPins->uiStorageLatchPin);
    gpio_set_dir(psPins->uiStorageLatchPin, GPIO_OUT);

    gpio_init(psPins->uiDataOutPin);
    gpio_set_dir(psPins->uiDataOutPin, GPIO_OUT);
}


void shift_out(S595OutPins* psPins, uint16_t uiData)
{
    int         i;
    bool        bBit;

    for (i = 0; i < 16; i++)
    {
        bBit = (uiData & 0x8000) == 0x8000;
        uiData = uiData << 1;
        gpio_put(psPins->uiShiftPin, false);
        gpio_put(psPins->uiDataOutPin, bBit);
        gpio_put(psPins->uiShiftPin, true);
    }
    gpio_put(psPins->uiShiftPin, false);

    gpio_put(psPins->uiStorageLatchPin, false);
    gpio_put(psPins->uiStorageLatchPin, true);
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


void init_shift(S165InPins* psPins)
{
    gpio_init(psPins->uiLatchPin);
    gpio_set_dir(psPins->uiLatchPin, GPIO_OUT);

    gpio_init(psPins->uiClockPin);
    gpio_set_dir(psPins->uiClockPin, GPIO_OUT);

    gpio_init(psPins->uiDataInPin);
    gpio_set_dir(psPins->uiDataInPin, GPIO_IN);
}


uint16_t shift_in(S165InPins* psPins)
{
    int         i;
    bool        bBit;
    uint16_t    uiData;
    int         iSleep;

    iSleep = 10;
    gpio_put(psPins->uiLatchPin, psPins->bLatchHigh);
    busy_wait_us_32(iSleep);
    gpio_put(psPins->uiLatchPin, !psPins->bLatchHigh);
    busy_wait_us_32(iSleep);

    uiData = 0;
    for (i = 0; i < 16; i++)
    {
        uiData = uiData << 1;

        gpio_put(psPins->uiClockPin, psPins->bDataOnClockLow);
        busy_wait_us_32(iSleep);
        gpio_put(psPins->uiClockPin, !psPins->bDataOnClockLow);
        bBit = gpio_get(psPins->uiDataInPin);
        uiData = uiData | (bBit ? !psPins->bDataInverted : psPins->bDataInverted);
        busy_wait_us_32(iSleep);
    }
     gpio_put(psPins->uiClockPin, psPins->bDataOnClockLow);
    return uiData;
}


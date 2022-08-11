#ifndef __SHIFT_REGISTER_H__
#define __SHIFT_REGISTER_H__
#include "pico/stdlib.h"


struct S595OutPins
{
    uint uiOutputEnableBPin;
    uint uiShiftPin;
    uint uiStorageLatchPin;
    uint uiDataOutPin;
    uint uiMasterResetPin;

    void Init(uint uiOutputEnableBPin,
              uint uiShiftPin,
              uint uiStorageLatchPin,
              uint uiDataOutPin,
              uint uiMasterResetPin);
};


struct S165InPins
{
    uint uiLatchPin;
    uint uiClockPin;
    uint uiDataInPin;

    bool bLatchHigh;
    bool bDataInverted;
    bool bDataOnClockLow;

    void Init(uint uiLatchPin, 
              uint uiClockPin,
              uint uiDataInPin,
              bool bLatchHigh,
              bool bDataInverted,
              bool bDataOnClockLow);
};


void init_shift(S595OutPins* psPins);
void shift_out(S595OutPins* psPins, uint16_t uiData);

void init_shift(S165InPins* psPins);
uint16_t shift_in(S165InPins* psPins);


#endif // __SHIFT_REGISTER_H__


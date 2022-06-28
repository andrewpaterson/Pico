#ifndef __SHIFT_REGISTER_H__
#define __SHIFT_REGISTER_H__
#include "pico/stdlib.h"


struct S595OutPins
{
    uint uiOutputEnableBPin;
    uint uiShiftPin;
    uint uiStorageLatchPin;
    uint uiDataInPin;
    uint uiMasterResetPin;

    void Init(uint uiOutputEnableBPin,
              uint uiShiftPin,
              uint uiStorageLatchPin,
              uint uiDataInPin,
              uint uiMasterResetPin);
};


void init_shift(S595OutPins* psPins);
void shift_out(S595OutPins* psPins, uint16_t uiData);


#endif // __SHIFT_REGISTER_H__


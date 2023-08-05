#ifndef __PICO_SOUND_H__
#define __PICO_SOUND_H__
#include "GeneralPins.h"


struct SPicoSound
{
    int iLeftEnablePin = 0;
    int iRightEnablePin = 1;
    int iStatusEnablePin = 15;
    int iSDCardEnablePin = 14;
    int aiEnablePins[4];
    int aiDataPins[8];

    uint32_t iFETEnableMask;
    uint32_t iFETDataMask;
    uint32_t iFETAllMask;
    int iLeftEnableMask;
    int iRightEnableMask;
    int iStatusEnableMask;
    int iSDCardEnableMask;
};
 

void init_sound(SPicoSound* psSound, 
                        int iLeftEnablePin,
                        int iRightEnablePin,
                        int iStatusEnablePin,
                        int iSDCardEnablePin,
                        int iData0Pin,
                        int iData1Pin,
                        int iData2Pin,
                        int iData3Pin,
                        int iData4Pin,
                        int iData5Pin,
                        int iData6Pin,
                        int iData7Pin);

void sound_disable_fets(SPicoSound* psSound);
void sound_set_data_in(SPicoSound* psSound);
void sound_write_left(SPicoSound* psSound, uint32_t iValue);
void sound_write_right(SPicoSound* psSound, uint32_t iValue);


#endif // __PICO_SOUND_H__


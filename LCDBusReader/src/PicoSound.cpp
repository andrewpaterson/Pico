#include "PicoSound.h"


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
                        int iData7Pin)
{
    psSound->aiEnablePins = {iLeftEnablePin, iRightEnablePin, iStatusEnablePin, iSDCardEnablePin};
    psSound->aiDataPins = {iData0Pin, iData1Pin, iData2Pin, iData3Pin, iData4Pin, iData5Pin, iData6Pin, iData7Pin};
    psSound->iLeftEnablePin = iLeftEnablePin;
    psSound->iRightEnablePin = iRightEnablePin;
    psSound->iStatusEnablePin = iStatusEnablePin;
    psSound->iSDCardEnablePin = iSDCardEnablePin;

    psSound->iFETEnableMask = make_4bit_mask(psSound->aiEnablePins , 0xf);
    psSound->iFETDataMask = make_8bit_mask(psSound->aiDataPins, 0xff);
    psSound->iFETAllMask = psSound->iFETEnableMask | psSound->iFETDataMask;

    psSound->iLeftEnableMask = 1ul << iLeftEnable;
    psSound->iRightEnableMask = 1ul << iRightEnable; 
    psSound->iStatusEnableMask = 1ul << iStatusEnable;
    
    gpio_init_mask(psSound->iFETAllMask);

    gpio_set_dir_masked(psSound->iFETAllMask, psSound->iFETAllMask);
}


void sound_disable_fets(SPicoSound* psSound)
{
    gpio_set_dir_masked(psSound->iFETAllMask, psSound->iFETEnableMask);
}


void sound_set_data_in(SPicoSound* psSound)
{
    gpio_put_masked(psSound->iFETEnableMask, 0);
}


void sound_set_data_out(SPicoSound* psSound)
{
    gpio_put_masked(psSound->iFETEnableMask, 0);
}


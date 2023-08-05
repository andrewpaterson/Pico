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
    psSound->aiEnablePins[0] = iLeftEnablePin;
    psSound->aiEnablePins[1] = iRightEnablePin;
    psSound->aiEnablePins[2] = iStatusEnablePin;
    psSound->aiEnablePins[3] = iSDCardEnablePin;

    psSound->aiDataPins[0] = iData0Pin;
    psSound->aiDataPins[1] = iData1Pin;
    psSound->aiDataPins[2] = iData2Pin;
    psSound->aiDataPins[3] = iData3Pin;
    psSound->aiDataPins[4] = iData4Pin;
    psSound->aiDataPins[5] = iData5Pin;
    psSound->aiDataPins[6] = iData6Pin;
    psSound->aiDataPins[7] = iData7Pin;

    psSound->iLeftEnablePin = iLeftEnablePin;
    psSound->iRightEnablePin = iRightEnablePin;
    psSound->iStatusEnablePin = iStatusEnablePin;
    psSound->iSDCardEnablePin = iSDCardEnablePin;

    psSound->iFETEnableMask = make_4bit_mask(psSound->aiEnablePins , 0xf);
    psSound->iFETDataMask = make_8bit_mask(psSound->aiDataPins, 0xff);
    psSound->iFETAllMask = psSound->iFETEnableMask | psSound->iFETDataMask;

    psSound->iLeftEnableMask = 1ul << psSound->iLeftEnablePin;
    psSound->iRightEnableMask = 1ul << psSound->iRightEnablePin; 
    psSound->iStatusEnableMask = 1ul << psSound->iStatusEnablePin;
    psSound->iSDCardEnableMask = 1ul << psSound->iSDCardEnableMask;    
    
    gpio_init_mask(psSound->iFETAllMask);

    sound_disable_fets(psSound);
}


void sound_disable_fets(SPicoSound* psSound)
{
    gpio_set_dir_masked(psSound->iFETAllMask, psSound->iFETEnableMask);
    gpio_put_masked(psSound->iFETEnableMask, 0);
}


void sound_set_data_in(SPicoSound* psSound)
{
    gpio_put_masked(psSound->iFETEnableMask, 0);
}


void sound_write_left(SPicoSound* psSound, uint32_t iValue)
{
    int iWriteLeft = psSound->iLeftEnableMask | make_8bit_mask(psSound->aiDataPins, iValue);
    gpio_set_dir_masked(psSound->iFETAllMask, psSound->iFETAllMask);
    gpio_put_masked(psSound->iFETAllMask , iWriteLeft);
}


void sound_write_right(SPicoSound* psSound, uint32_t iValue)
{
    int iWriteRight = psSound->iRightEnableMask | make_8bit_mask(psSound->aiDataPins, iValue);
    gpio_set_dir_masked(psSound->iFETAllMask, psSound->iFETAllMask);
    gpio_put_masked(psSound->iFETAllMask , iWriteRight);
}


uint32_t sound_read_status(SPicoSound* psSound)
{
    gpio_set_dir_masked(psSound->iFETAllMask, psSound->iFETEnableMask);
    gpio_put_masked(psSound->iFETEnableMask, psSound->iStatusEnableMask);
    uint32_t iPins = gpio_get_all();
    iPins = iPins & psSound->iFETDataMask;
    iPins >>= 2;
    return iPins;
}


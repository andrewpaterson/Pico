#ifndef __PICO_SOUND_H__
#define __PICO_SOUND_H__

struct SPicoSound
{
    int iLeftEnablePin = 0;
    int iRightEnablePin = 1;
    int iStatusEnablePin = 15;
    int iSDCardEnablePin = 14;
    int aiEnablePins[4];
    int aiDataPins[] = {2,3,4,5,6,7,8,9};
}


void init_pico_sound(SPicoSound* psSound, 
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
    aiEnablePins = {iLeftEnablePin, iRightEnablePin, iStatusEnablePin, iSDCardEnablePin};
    aiDataPins = {iData0Pin, iData1Pin, iData2Pin, iData3Pin, iData4Pin, iData5Pin, iData6Pin, iData7Pin};
    this->iLeftEnablePin = iLeftEnablePin;
    this->iRightEnablePin = iRightEnablePin;
    this->iStatusEnablePin = iStatusEnablePin;
    this->iSDCardEnablePin = iSDCardEnablePin;
}


#endif // __PICO_SOUND_H__
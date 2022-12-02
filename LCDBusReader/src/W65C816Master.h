#ifndef __W65C816_MASTER_H__
#define __W65C816_MASTER_H__
#include "W65C816Pins.h"
#include "W65C816Bus.h"


class CW65C816Master : CW65C816Bus
{
protected:
    int             miBank;
    uart_inst_t*    mpUart;
    int             miPinResB;
    int             miPinNmiB;
    int             miPinIrqB;
    int             miPinAbortB;
    int             miPinBE;
    int             miPinWEB;
    int             miPinOEB;
    int             miPinClkSel;
    int             miPinClk;

public:
    void    Init(SW65C816Pins* psPins, uart_inst_t* pUart, int iPinResB, int iPinNmiB, int iPinIrqB, int iPinAbortB, int iPinBE, int iPinWEB, int iPinOEB, int iPinClkSel, int iPinClk);

    bool    Write(uint uiAddress, uint uiData);
    int     Read(uint uiAddress);
    bool    HighZ(void);

    void    Reset(bool bReset);
    void    BusEnable(bool bEnable);
    void    FreeClock(bool bFreeRunningClock);

protected:
    void    InitSignal(int iPin, bool bValue);
    void    AddressToString(char* szDest, uint uiAddress);
    void    DataToString(char* szDest, uint uiData);
    bool    SendMessage(char* szMessage);
    int     SendMessageReceiveByte(char* szMessage);
};


#endif // __W65C816_SLAVE_H__


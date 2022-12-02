#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "UARTComm.h"
#include "W65C816Master.h"


void CW65C816Master::Init(SW65C816Pins* psPins, uart_inst_t* pUart, int iPinResB, int iPinNmiB, int iPinIrqB, int iPinAbortB, int iPinBE, int iPinWEB, int iPinOEB, int iPinClkSel, int iPinClk)
{
    CW65C816Bus::Init(psPins);
    miBank = 0;
    mpUart = pUart;

    miPinResB = iPinResB;
    miPinNmiB = iPinNmiB;
    miPinIrqB = iPinIrqB;
    miPinAbortB = iPinAbortB;
    miPinBE = iPinBE;
    miPinWEB = iPinWEB;
    miPinOEB = iPinOEB;
    miPinClkSel = iPinClkSel;
    miPinClk = iPinClk;

    InitSignal(miPinBE, false);
    InitSignal(miPinResB, false);
    InitSignal(miPinNmiB, true);
    InitSignal(miPinIrqB, true);
    InitSignal(miPinAbortB, true);

    InitSignal(miPinWEB, true);
    InitSignal(miPinOEB, true);
    InitSignal(miPinClkSel, true);
    InitSignal(miPinClk, false);
}


void CW65C816Master::InitSignal(int iPin, bool bValue)
{
    gpio_init(iPin);
    gpio_set_dir(iPin, GPIO_OUT);
    gpio_put(iPin, bValue);
}


bool CW65C816Master::SendMessage(char* szMessage)
{
    bool    bResult;
    char    szResponse[256];

    for (int iRetry = 0; iRetry < 10; iRetry++)
    {
        bResult = write_uart_message(mpUart, szMessage);
        if (!bResult)
        {
            return false;
        }

        for (int i = 0; i < 100; i++)
        {
            bResult = read_uart_message(szResponse);
            if (bResult)
            {
                break;
            }
        }

        if (bResult)
        {
            if (strcmp(szResponse, "OK") == 0)
            {
                return true;
            }
        }
    }

    return false;
}


int CW65C816Master::SendMessageReceiveByte(char* szMessage)
{
    bool    bResult;
    char    szResponse[256];

    for (int iRetry = 0; iRetry < 10; iRetry++)
    {
        bResult = write_uart_message(mpUart, szMessage);
        if (!bResult)
        {
            return -1;
        }

        for (int i = 0; i < 100; i++)
        {
            bResult = read_uart_message(szResponse);
            if (bResult)
            {
                break;
            }
        }

        if (bResult)
        {
            long iData = strtol(szResponse, NULL, 16);
            if (iData >= 0 && iData <= 0xFF)
            {
                return iData;
            }
        }
    }

    return -1;
}


void CW65C816Master::AddressToString(char* szDest, uint uiAddress)
{
    itoa(uiAddress, szDest, 16);
    int iLen = strlen(szDest);
    int iPad = 4 - iLen;
    memmove(&szDest[2 + iPad], szDest, iLen);
    for (int i = 0; i < iPad; i++)
    {
        szDest[2 + i] = '0';
    }
    szDest[0] = 'A';
    szDest[1] = ':';
    szDest[6] = '\n';
    szDest[7] = '\0';
}


void CW65C816Master::DataToString(char* szDest, uint uiData)
{
    itoa(uiData, szDest, 16);
    int iLen = strlen(szDest);
    int iPad = 2 - iLen;
    memmove(&szDest[2 + iPad], szDest, iLen);
    for (int i = 0; i < iPad; i++)
    {
        szDest[2 + i] = '0';
    }
    szDest[0] = 'D';
    szDest[1] = ':';
    szDest[4] = '\n';
    szDest[5] = '\0';
}


bool CW65C816Master::Write(uint uiAddress, uint uiData)
{
    char    sz[32];

    gpio_put(miPinBE, false);
    gpio_put(miPinOEB, true);
    gpio_put(miPinWEB, true);

    AddressToString(sz, uiAddress & 0xFFFF);
    SendMessage(sz);
    DataToString(sz, uiData & 0xFF);
    SendMessage(sz);
    AddressOutDataOut(uiData, uiAddress);
    SendMessage("IO:D+A\n");

    gpio_put(miPinWEB, false);
    gpio_put(miPinWEB, false);
    gpio_put(miPinWEB, false);
    gpio_put(miPinWEB, true);
}


int CW65C816Master::Read(uint uiAddress)
{
    char    sz[32];
    int     iData;

    gpio_put(miPinBE, false);
    gpio_put(miPinOEB, true);
    gpio_put(miPinWEB, true);

    AddressToString(sz, uiAddress & 0xFFFF);
    SendMessage(sz);
    AddressOutDataIn(uiAddress);
    SendMessage("IO:A\n");

    gpio_put(miPinOEB, false);
    iData = SendMessageReceiveByte("READ");
    gpio_put(miPinOEB, true);
    return iData;
}


bool CW65C816Master::HighZ(void)
{
    DisableIO();
    return SendMessage("IO:Z\n");
}


void CW65C816Master::Reset(bool bReset)
{
    gpio_put(miPinResB, !bReset);
}


void CW65C816Master::BusEnable(bool bEnable)
{
    gpio_put(miPinBE, bEnable);
}


void CW65C816Master::FreeClock(bool bFreeRunningClock)
{
    gpio_put(miPinClkSel, !bFreeRunningClock);
}


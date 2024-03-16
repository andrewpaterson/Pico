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


bool CW65C816Master::SendMessage(const char* szMessage, bool bIgnoreResponse)
{
    bool    bResult;
    char    szResponse[256];

    if (bIgnoreResponse)
    {
        write_uart_message(mpUart, szMessage);
        return true;
    }
    else
    {
        for (int iRetry = 0; iRetry < 10; iRetry++)
        {
            bResult = write_uart_message(mpUart, szMessage);
            if (!bResult)
            {
                return false;
            }

            for (int i = 0; i < 10; i++)
            {
                bResult = read_uart_message(szResponse);
                if (bResult)
                {
                    break;
                }
                busy_wait_us_32(1);
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
}


int CW65C816Master::SendMessageReceiveByte(const char* szMessage)
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

        for (int i = 0; i < 10; i++)
        {
            bResult = read_uart_message(szResponse);
            if (bResult)
            {
                break;
            }
            busy_wait_us_32(1);
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


bool CW65C816Master::SendAddress(uint uiAddress, bool bIgnoreResponse)
{
    char    sz[32];

    AddressToString(sz, uiAddress & 0xFFFF);
    return SendMessage(sz, bIgnoreResponse);
}


bool CW65C816Master::SendData(uint uiData, bool bIgnoreResponse)
{
    char    sz[32];

    DataToString(sz, uiData & 0xFF);
    return SendMessage(sz, bIgnoreResponse);
}


bool CW65C816Master::SendAddressOutDataOut(bool bIgnoreResponse)
{
    return SendMessage("IO:D+A\n", bIgnoreResponse);
}


bool CW65C816Master::Write(uint uiAddress, uint uiData, bool bIgnoreResponse)
{
    char    sz[32];
    bool    bResult;

    AddressOutDataOut(uiData, uiAddress);
    bResult = SendAddress(uiAddress, bIgnoreResponse);
    if (bResult)
    {
        bResult = SendData(uiData, bIgnoreResponse);
        if (bResult)
        {
            bResult = SendAddressOutDataOut(bIgnoreResponse);
            return bResult;
        }
    }
    return false;
}


int CW65C816Master::Read(uint uiAddress)
{
    char    sz[32];
    int     iData;
    bool    bResult;

    gpio_put(miPinBE, false);
    gpio_put(miPinOEB, true);
    gpio_put(miPinWEB, true);

    AddressToString(sz, uiAddress & 0xFFFF);
    SendMessage(sz);
    AddressOutDataIn(uiAddress);
    SendMessage("IO:A\n");

    gpio_put(miPinOEB, false);
    iData = SendMessageReceiveByte("READ\n");
    gpio_put(miPinOEB, true);
    return iData;
}


bool CW65C816Master::HighZ(bool bIgnoreResponse)
{
    DisableIO();
    return SendMessage("IO:Z\n", bIgnoreResponse);
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


void CW65C816Master::Tick(bool bPhi2)
{
    gpio_put(miPinClk, bPhi2);
}


void CW65C816Master::SramOutputEnable(bool bEnable)
{
    gpio_put(miPinOEB, !bEnable);
}


void CW65C816Master::SramWriteEnable(bool bEnable)
{
    gpio_put(miPinWEB, !bEnable);
}


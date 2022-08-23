#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "LCDDisplay.h"
#include "ShiftRegister.h"
#include "ShiftLCDDisplay.h"
#include "HEXDisplay.h"
#include "GeneralPins.h"
#include "UARTComm.h"
#include "SPIComm.h"
#include "LTC6903.h"


void kitt(int iCount, char* szDest)
{
    int iMod = iCount % 26;
    int i;

    if (iMod <= 13)
    {
        for (i = 0; i < 16; i++)
        {
            if ((i >= iMod) && (i <= iMod + 2))
            {
                szDest[i] = '-';
            }
            else
            {
                szDest[i] = ' ';
            }
        }
    }
    else
    {
        iMod = 26 - iMod;
        for (i = 0; i < 16; i++)
        {
            if ((i >= iMod) && (i <= iMod + 2))
            {
                szDest[i] = '-';
            }
            else
            {
                szDest[i] = ' ';
            }
        }
    }
    szDest[16] = 0;
}


void message(int iCount, char* szDest)
{
    if (((iCount / 20) % 2) == 0)
    {
        strcpy(szDest, "  Parallel pins ");
    }
    else
    {
        strcpy(szDest, "   driven LCD   ");
    }
}


void shift_LCD(void)
{
    S595OutPins sPins;
    sPins.Init(PIN_NOT_SET, 18, 17, 16, PIN_NOT_SET);
    init_shift(&sPins);

    init_lcd(&sPins);
    put_clear(&sPins);

    int  i = 0;
    char szLine1[17];
    char szLine2[17];
    while(true)
    {
        kitt(i, szLine1);
        message(i, szLine2);

        put_lines(&sPins, szLine1, szLine2);

        sleep_us_high_power(200000);

        i++;
    }
}


void parallel_LCD(void)
{
    bool led = true;

    S11BitLCDPins   sPins;

    sPins.Init(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

    init_lcd(&sPins);
    put_clear(&sPins);

    int  i = 0;
    char szLine1[17];
    char szLine2[17];
    while(true)
    {
        kitt(i, szLine1);
        message(i, szLine2);

        put_lines(&sPins, szLine1, szLine2);

        sleep_us_high_power(200000);

        i++;
    }
}

int get_snes_button(uint16_t uiButtons)
{
    if (uiButtons & 0x8000)
    {
        return 1;
    }
    if (uiButtons & 0x4000)
    {
        return 2;
    }
    if (uiButtons & 0x2000)
    {
        return 3;
    }
    if (uiButtons & 0x1000)
    {
        return 4;
    }
    if (uiButtons & 0x800)
    {
        return 5;
    }
    if (uiButtons & 0x400)
    {
        return 6;
    }
    if (uiButtons & 0x200)
    {
        return 7;
    }
    if (uiButtons & 0x100)
    {
        return 8;
    }
    if (uiButtons & 0x80)
    {
        return 9;
    }
    if (uiButtons & 0x40)
    {
        return 10;
    }
    if (uiButtons & 0x20)
    {
        return 11;
    }
    if (uiButtons & 0x10)
    {
        return 12;
    }

    return 0;
}

int     giDelay;


int make_sd_data_mask(  int iSDDat0Pin,
                        int iSDDat1Pin,
                        int iSDDat2Pin,
                        int iSDDat3Pin,
                        int iData)
{
    int iMask = (iData & 0x01 ? (1 << iSDDat0Pin) : 0) | 
                (iData & 0x02 ? (1 << iSDDat1Pin) : 0) | 
                (iData & 0x04 ? (1 << iSDDat2Pin) : 0) | 
                (iData & 0x08 ? (1 << iSDDat3Pin) : 0);

    return iMask;
}

static uint8_t crc7(uint8_t uiCRC, uint8_t uiData) 
{
	uiCRC ^= uiData;
	for (uint8_t i = 0; i < 8; i++) 
    {
		if (uiCRC & 0x80) 
        {
            uiCRC ^= 0x89;
        }
		uiCRC <<= 1;
	}

	return uiCRC;
}

uint8_t crc7(uint8_t* pvData, int iLength)
{
	uint8_t uiCRC = 0;

	for (int i = 0; i < iLength; i++) 
    {
        uiCRC = crc7(uiCRC, pvData[i]);
    }

	return uiCRC >> 1;
}

static uint16_t crc16(uint16_t uiCRC, uint8_t uiData) 
{
	uiCRC  = (uint8_t)(uiCRC >> 8) | (uiCRC << 8);
	uiCRC ^= uiData;
	uiCRC ^= (uint8_t)(uiCRC & 0xff) >> 4;
	uiCRC ^= (uiCRC << 8) << 4;
	uiCRC ^= ((uiCRC & 0xff) << 4) << 1;

	return uiCRC;
}

uint16_t crc16(uint8_t* pvData, uint32_t len) 
{
	uint16_t uiCRC = 0;

	for (int i = 0; i < len; i++) 
    {
        uiCRC = crc16(uiCRC, pvData[i]);
    }

	return uiCRC;
}

void build_command(uint8_t* pvDest, int iCommand, int iArgument)
{
    pvDest[0] = 0x40 | (iCommand & 0x3f);  //0..1...command[5..0]...
    pvDest[1] = (iArgument >> 24) & 0xff;  //argument[31..24]...
    pvDest[2] = (iArgument >> 16) & 0xff;  //argument[23..16]...
    pvDest[3] = (iArgument >> 8) & 0xff;   //argument[15..8]...
    pvDest[4] = iArgument & 0xff;          //argument[7..0]...
    uint8_t uiCRC = crc7(pvDest, 5);
    pvDest[5] = (uiCRC << 1) | 1;
}

bool send_command(uint8_t* pvDest, int iSDClkPin, int iSDCmdPin)
{
    gpio_set_dir(iSDCmdPin, GPIO_OUT);
    gpio_put(iSDClkPin, false);
    gpio_put(iSDCmdPin, true);
    sleep_us_high_power(5);
    for (int iByte = 0; iByte <= 5; iByte++)
    {
        uint8_t uiByte = pvDest[iByte];
        for (int iBit = 7; iBit >=0; iBit--)
        {
            bool bBit = uiByte & 0x80;
            uiByte <<= 1;
            gpio_put(iSDClkPin, false);
            gpio_put(iSDCmdPin, bBit);
            sleep_us_high_power(5);
            gpio_put(iSDClkPin, true);
            sleep_us_high_power(5);
        }
    }
}

bool read_response(int iSDClkPin, int iSDCmdPin, int iExpectedBytes, uint8_t* pvResponse)
{
    int iBit = 6;
    for (int iByte = 0; iByte < iExpectedBytes; iByte++)
    {
        pvResponse[iByte] = 0;
        for (; iBit >= 0; iBit--)
        {
            pvResponse[iByte] <<= 1;
            gpio_put(iSDClkPin, false);
            sleep_us_high_power(5);
            gpio_put(iSDClkPin, true);
            bool bBit = gpio_get(iSDCmdPin);
            sleep_us_high_power(5);
            pvResponse[iByte] |= bBit;
        }
        iBit = 7;
    }
}

void sd_clock_tick(int iSDClkPin, int iSDCmdPin, int iFullCycles, int iCmdDir)
{
    if (gpio_get_dir(iSDCmdPin) != iCmdDir)
    {
        gpio_set_dir(iSDCmdPin, iCmdDir);
    }

    if (iCmdDir == GPIO_OUT)
    {
        gpio_put(iSDCmdPin, true);
    }

    for (int i = 0; i < iFullCycles; i++)
    {
        gpio_put(iSDClkPin, false);
        sleep_us_high_power(5);
        gpio_put(iSDClkPin, true);
        sleep_us_high_power(5);
    }
}

bool receive_response(int iSDClkPin, int iSDCmdPin, int iExpectedBytes, uint8_t* pvResponse)
{
    if (gpio_get_dir(iSDCmdPin) == GPIO_OUT)
    {
        gpio_set_dir(iSDCmdPin, GPIO_IN);
    }

    bool bCardTransmit = false;
    for (int i = 0; i < 200; i++)
    {
        gpio_put(iSDClkPin, false);
        sleep_us_high_power(5);
        gpio_put(iSDClkPin, true);
        bool bBit = gpio_get(iSDCmdPin);
        sleep_us_high_power(5);
        if (!bBit)
        {
            bCardTransmit = true;
            break;
        }
    }

    if (bCardTransmit)
    {
        bool bValidResponse = read_response(iSDClkPin, iSDCmdPin, iExpectedBytes, pvResponse);
        if (bValidResponse)
        {
            sd_clock_tick(iSDClkPin, iSDCmdPin, 8, GPIO_OUT);
        }
        return bValidResponse;
    }
    else
    {
        return false;
    }
}

void sd_cmd0_go_idle(int iSDClkPin, int iSDCmdPin, int iSDDat3Pin, bool bSDMode)
{
    uint8_t aCommand[6];

    build_command(aCommand, 0, 0);
    
    gpio_set_dir(iSDDat3Pin, GPIO_OUT);
    gpio_put(iSDDat3Pin, !bSDMode);

    sd_clock_tick(iSDClkPin, iSDCmdPin, 74, GPIO_OUT);

    send_command(aCommand, iSDClkPin, iSDCmdPin);

    gpio_set_dir(iSDDat3Pin, GPIO_IN);

    sd_clock_tick(iSDClkPin, iSDCmdPin, 8, GPIO_OUT);
}


struct SSDResponseR1
{
    uint8_t uiCmd;  //Bit 7   : 0 (Start transmit)
                    //Bit 6   : 0 (Card transmit)
                    //Bit 5..0: cmd

    uint8_t uiStatus3;
    uint8_t uiStatus2;
    uint8_t uiStatus1;
    uint8_t uiStatus0;

    uint8_t uiCrc7;  //Bit 6..1: CRC7
                     //Bit 0   : 1 (Stop transmit)
};


bool sd_cmd8_interface_condition(int iSDClkPin, int iSDCmdPin)
{
    uint8_t aCommand[6];
    uint8_t aResponse[6];
    bool bResult;

    build_command(aCommand, 8, 0x000001AA);
    send_command(aCommand, iSDClkPin, iSDCmdPin);
    sd_clock_tick(iSDClkPin, iSDCmdPin, 4, GPIO_IN);

    bResult = receive_response(iSDClkPin, iSDCmdPin, 6, aResponse);
    if (bResult)
    {
        SSDResponseR1* pResponse = (SSDResponseR1*)aResponse;
        if ((pResponse->uiCmd == 8) &&
            (pResponse->uiStatus1 == 1) &&
            (pResponse->uiStatus0 == 0xAA) &&
            (pResponse->uiCrc7 == 0x13))
        {
            return true;
        }
    }
    return false;
}


bool sd_cmd55_application_specific(int iSDClkPin, int iSDCmdPin, u_int16_t uiRCA)
{
    uint8_t aCommand[6];
    uint8_t aResponse[6];
    bool bResult;

    build_command(aCommand, 55, uiRCA << 1);
    send_command(aCommand, iSDClkPin, iSDCmdPin);

    sd_clock_tick(iSDClkPin, iSDCmdPin, 4, GPIO_IN);

    bResult = receive_response(iSDClkPin, iSDCmdPin, 6, aResponse);
    if (bResult)
    {   
        SSDResponseR1* pResponse = (SSDResponseR1*)aResponse;
        if ((pResponse->uiCmd == 55) &&
            (pResponse->uiStatus1 == 1) &&
            (pResponse->uiStatus0 == 0x20) &&
            (pResponse->uiCrc7 == 0x83))
        {
            return true;
        }
    }
    return false;
}


struct SSDResponseR3
{
    uint8_t uiReserved1;  //Bit 7   : 0 (Start transmit)
                          //Bit 6   : 0 (Card transmit)
                          //Bit 5..0: 0b111111

    uint8_t uiOCR3;
    uint8_t uiOCR2;
    uint8_t uiOCR1;
    uint8_t uiOCR0;

    uint8_t uiReserved0;  //Bit 6..1: 0b1111111
                          //Bit 0   : 1 (Stop transmit)
};


bool sd_cmd41_operating_condition(int iSDClkPin, int iSDCmdPin, uint8_t* paResponse)
{
    uint8_t aCommand[6];
    bool bResult;

    build_command(aCommand, 41, 0x40100000);
    send_command(aCommand, iSDClkPin, iSDCmdPin);
    sd_clock_tick(iSDClkPin, iSDCmdPin, 4, GPIO_IN);

    bResult = receive_response(iSDClkPin, iSDCmdPin, 6, paResponse);
    if (bResult)
    {
        SSDResponseR3* pResponse = (SSDResponseR3*)paResponse;
        if ((pResponse->uiReserved1 == 0x3F) &&
            (pResponse->uiReserved0 == 0xFF))
        {
            return true;
        }
    }
    return false;
}


struct SSDOCR
{
    bool b27_28Volts;
    bool b28_29Volts;
    bool b29_30Volts;
    bool b30_31Volts;
    bool b31_32Volts;
    bool b32_33Volts;
    bool b33_34Volts;
    bool b34_35Volts;
    bool b35_36Volts;
    bool bSwitchTo18Volts;     //S18A
    bool bOver2TBytes;         //CO2T
    bool bUHSIIStatus;
    bool bCardCapacityStatus;  //CCS
};


bool sd_acmd41_application_operating_condition(int iSDClkPin, int iSDCmdPin, u_int16_t uiRCA, SSDOCR* pOCR)
{
    uint8_t aResponse[6];
    bool bResult;

    for (int i = 0; i < 10; i++)
    {
        bResult = sd_cmd55_application_specific(iSDClkPin, iSDCmdPin, 0);
        if (!bResult)
        {
            return false;
        }

        bResult = sd_cmd41_operating_condition(iSDClkPin, iSDCmdPin, aResponse);
        if (!bResult)
        {
            return false;
        }

        SSDResponseR3* pResponse = (SSDResponseR3*)aResponse;
        if ((pResponse->uiOCR3 & 0x80) == 0x80)
        {
            pOCR->b35_36Volts = (pResponse->uiOCR2 & 0x80) == 0x80;
            pOCR->b34_35Volts = (pResponse->uiOCR2 & 0x40) == 0x40;
            pOCR->b33_34Volts = (pResponse->uiOCR2 & 0x20) == 0x20;
            pOCR->b32_33Volts = (pResponse->uiOCR2 & 0x10) == 0x10;
            pOCR->b31_32Volts = (pResponse->uiOCR2 & 0x8) == 0x8;
            pOCR->b30_31Volts = (pResponse->uiOCR2 & 0x4) == 0x4;
            pOCR->b29_30Volts = (pResponse->uiOCR2 & 0x2) == 0x2;
            pOCR->b28_29Volts = (pResponse->uiOCR2 & 0x1) == 0x1;
            pOCR->b27_28Volts = (pResponse->uiOCR1 & 0x80) == 0x80;

            pOCR->bSwitchTo18Volts = (pResponse->uiOCR3 & 0x1) == 0x1;
            pOCR->bOver2TBytes = (pResponse->uiOCR3 & 0x8) == 0x8;
            pOCR->bUHSIIStatus = (pResponse->uiOCR3 & 0x20) == 0x20;
            pOCR->bCardCapacityStatus = (pResponse->uiOCR3 & 0x40) == 0x40;
            return true;
        }
    }
    return false;
}


struct SSDResponseR2
{
    uint8_t     uiReserved; //Bit 7   : 0 (Start transmit)
                            //Bit 6   : 0 (Card transmit)
                            //Bit 5..0: 0b111111

    uint8_t     uiManufacturerID;
    uint16_t    uiApplicationID;
    char        acProductName[5];
    uint8_t     uiProductRevision;
    uint8_t     auiProductSerialNumber[4];

    uint16_t    uiManufactureDate;  //Bit 15..12: 0bXXXX
                                    //Bit 11..4 : Year (+2000)
                                    //Bit 3..0  : Month

    uint8_t uiCrc7;  //Bit 6..1: CRC7
                     //Bit 0   : 1 (Stop transmit)
};


struct SSDCID
{
    uint8_t     uiManufacturerID;
    uint16_t    uiApplicationID;
    char        szProductName[6];
    uint8_t     uiProductRevision;
    int32_t     iProductSerialNumber;

    uint16_t    uiManufactureYear;
    uint8_t     uiManufactureMonth;
};


bool sd_cmd2_send_cid_numbers(int iSDClkPin, int iSDCmdPin, SSDCID* pCID)
{
    uint8_t aCommand[6];
    uint8_t aResponse[17];
    bool bResult;

    build_command(aCommand, 2, 0);
    send_command(aCommand, iSDClkPin, iSDCmdPin);
    sd_clock_tick(iSDClkPin, iSDCmdPin, 4, GPIO_IN);

    bResult = receive_response(iSDClkPin, iSDCmdPin, 17, aResponse);
    if (bResult)
    {
        SSDResponseR2* pResponse = (SSDResponseR2*)aResponse;
        if (pResponse->uiReserved == 0x3F)
        {
            uint8_t uiExpectedCRC = crc7(&aResponse[1], 15);
            uiExpectedCRC <<= 1;
            uiExpectedCRC |= 1;
            if (pResponse->uiCrc7 == uiExpectedCRC)
            {
                pCID->uiManufacturerID = pResponse->uiManufacturerID;
                pCID->uiApplicationID = pResponse->uiApplicationID;
                pCID->uiProductRevision = pResponse->uiProductRevision;
                
                memcpy(pCID->szProductName, pResponse->acProductName, 5);
                pCID->szProductName[5] = '\0';
                
                pCID->iProductSerialNumber = *((int*)pResponse->auiProductSerialNumber);

                pCID->uiManufactureYear = ((pResponse->uiManufactureDate >> 4) & 0xFF) + 2000;
                pCID->uiManufactureMonth =  pResponse->uiManufactureDate & 0xF;

                return true;
            }
        }
    }
    return false;
}


struct SSDResponseR6
{
    uint8_t uiCmd;  //Bit 7   : 0 (Start transmit)
                    //Bit 6   : 0 (Card transmit)
                    //Bit 5..0: cmd

    uint8_t uiRCA1;
    uint8_t uiRCA0;
    uint8_t uiStatus1;  
    uint8_t uiStatus0;

    uint8_t uiCrc7;  //Bit 6..1: CRC7
                     //Bit 0   : 1 (Stop transmit)
};


enum ESDState
{
    ESD_Idle,
    ESD_Ready,
    ESD_Identify,
    ESD_StandBy,
    ESD_Transmit,
    ESD_Data,
    ESD_Receive,
    ESD_Program,
    ESD_Disabled,

    ESD_Reserved
};

struct SSDR6Status
{
    bool        bPreviousCRCFailed;
    bool        bIllegalCommand;
    bool        bGeneralError;

    ESDState    eState;

    bool        bReadyForData;
    bool        bExtensionFunctionEvent;
    bool        bApplicationCommandExpected;
    bool        bAuthenticationError;
};


bool sd_cmd3_publish_relative_address(int iSDClkPin, int iSDCmdPin, uint16_t* puiAddress, SSDR6Status* pStatus)
{
    uint8_t aCommand[6];
    uint8_t aResponse[6];
    bool bResult;

    build_command(aCommand, 3, 0);
    send_command(aCommand, iSDClkPin, iSDCmdPin);
    sd_clock_tick(iSDClkPin, iSDCmdPin, 4, GPIO_IN);

    bResult = receive_response(iSDClkPin, iSDCmdPin, 6, aResponse);
    if (bResult)
    {
        SSDResponseR6* pResponse = (SSDResponseR6*)aResponse;
        if (pResponse->uiCmd == 3)
        {
            uint8_t uiExpectedCRC = crc7(aResponse, 5);
            uiExpectedCRC <<= 1;
            uiExpectedCRC |= 1;
            if (pResponse->uiCrc7 == uiExpectedCRC)
            {
                *puiAddress = *((uint16_t*)&pResponse->uiRCA1);
                pStatus->bPreviousCRCFailed = (pResponse->uiStatus1 & 0x80) == 0x80;
                pStatus->bIllegalCommand = (pResponse->uiStatus1 & 0x40) == 0x40;
                pStatus->bGeneralError = (pResponse->uiStatus1 & 0x20) == 0x20;
                pStatus->eState = (ESDState)((pResponse->uiStatus1 & 0x1E) >> 1);
                
                pStatus->bReadyForData = (pResponse->uiStatus0 & 0x100) == 0x100;
                pStatus->bExtensionFunctionEvent = (pResponse->uiStatus0 & 0x40) == 0x40;
                pStatus->bApplicationCommandExpected = (pResponse->uiStatus0 & 0x20) == 0x20;
                pStatus->bAuthenticationError = (pResponse->uiStatus0 & 0x8) == 0x8;
                return true;
            }   
        }
    }
    return false;
}


void blink_led(int iMicrosecondDelay)
{
    bool bLed = true;

    for (;;)
    {
        gpio_put(25, bLed);
        sleep_us(iMicrosecondDelay);

        bLed = !bLed;
    }
}

int main() 
{
    stdio_init_all();
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);


    int iSDDat0Pin = 2;
    int iSDDat1Pin = 3;
    int iSDDat2Pin = 4;
    int iSDDat3Pin = 5;

    int iSDClkPin = 6;
    int iSDCmdPin = 7;

    gpio_init(iSDClkPin);
    gpio_set_dir(iSDClkPin, GPIO_OUT);

    gpio_init(iSDCmdPin);
    gpio_set_dir(iSDCmdPin, GPIO_OUT);

    int iSDMask = make_sd_data_mask(iSDDat0Pin, iSDDat1Pin, iSDDat2Pin, iSDDat3Pin, 0xf);

    gpio_init_mask(iSDMask);
    gpio_set_dir_in_masked(iSDMask);


    SSDOCR      sOCR;
    SSDCID      sCID;
    SSDR6Status sStatus;
    uint16_t    uiAddress;
    bool        bResult;

    sd_cmd0_go_idle(iSDClkPin, iSDCmdPin, iSDDat3Pin, false);

    bResult = sd_cmd8_interface_condition(iSDClkPin, iSDCmdPin);
    if (bResult)
    {
        bResult = sd_acmd41_application_operating_condition(iSDClkPin, iSDCmdPin, 0, &sOCR);
        if (bResult)
        {
            bResult = sd_cmd2_send_cid_numbers(iSDClkPin, iSDCmdPin, &sCID);
            if (bResult)
            {
                bResult = sd_cmd3_publish_relative_address(iSDClkPin, iSDCmdPin, &uiAddress, &sStatus);
            }
        }
        else
        {
            blink_led(100'000);
        }
    }
    else
    {
        blink_led(100'000);
    }
    blink_led(20'000);








    int iSlaveIO = 15;

    gpio_init(iSlaveIO);
    gpio_set_dir(iSlaveIO, GPIO_IN);

    uart_inst_t* pUart = init_uart(0, 1, 115200 * 4);
    int iIRQ = get_uart_irq(pUart);

    irq_set_exclusive_handler(iIRQ, uart_receive_irq);
    irq_set_enabled(iIRQ, true);
    uart_set_irq_enables(pUart, true, false);

    bool bSlave = gpio_get(iSlaveIO);
    if (bSlave)
    {
        giDelay = 1000000;
    }
    else
    {
        giDelay = 2000000;
    }

    if (!bSlave)
    {
        bool bLed = true;

        for (;;)
        {
            gpio_put(25, bLed);
            if (bLed)
            {
                uart_puts(pUart, "Fast\n");
            }
            else
            {
                uart_puts(pUart, "Slow\n");
            }
            
            int delay = giDelay;

            uint64_t start = time_us_64();
            uint64_t expectedEnd = start + delay;
            uint64_t end = start;

            while (expectedEnd > end)
            {
                end = time_us_64();
            }            
            bLed = !bLed;
        }
    }
    else
    {
        bool bLed = true;
        for (;;)
        {
            gpio_put(25, bLed);

            int delay = giDelay;

            uint64_t start = time_us_64();
            uint64_t expectedEnd = start + delay;
            uint64_t end = start;

            while (expectedEnd > end)
            {
                end = time_us_64();
                char szMessage[256];
                bool bNewMessage = read_uart_message(szMessage);
                if (bNewMessage)
                {
                    giDelay = 20000;
                    if (memcmp(szMessage, "Fast", 4) == 0)
                    {
                        giDelay = 20000;
                    }
                    else if (memcmp(szMessage, "Slow", 4) == 0)
                    {
                        giDelay = 50000;
                    }
                }
                sleep_us_high_power(1000);
            }            
            bLed = !bLed;
        }
    }
}


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

#define  SD_SLEEP 1

void blink_led(int iMicrosecondDelay)
{
    bool bLed = true;

    for (;;)
    {
        gpio_put(25, bLed);
        sleep_us_high_power(iMicrosecondDelay);

        bLed = !bLed;
    }
}

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


uint16_t crc16(uint8_t* pvData, int iLength) 
{
    uint16_t crc;
    uint8_t i;

    crc = 0;
	for (int iByte = 0; iByte < iLength; iByte++)
    {
        crc = crc ^ ((uint16_t) (pvData[iByte] << 8));
        for(int iBit = 0; iBit < 8; iBit++)
        {
            if( crc & 0x8000 )
            {
                crc = (crc << 1) ^ 0x1021;
            }
            else
            {
                crc = crc << 1;
            }
        }
    }

    return crc;
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
    sleep_us_high_power(SD_SLEEP);
    for (int iByte = 0; iByte <= 5; iByte++)
    {
        uint8_t uiByte = pvDest[iByte];
        for (int iBit = 7; iBit >=0; iBit--)
        {
            bool bBit = uiByte & 0x80;
            uiByte <<= 1;
            gpio_put(iSDClkPin, false);
            gpio_put(iSDCmdPin, bBit);
            sleep_us_high_power(SD_SLEEP);
            gpio_put(iSDClkPin, true);
            sleep_us_high_power(SD_SLEEP);
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
            sleep_us_high_power(SD_SLEEP);
            gpio_put(iSDClkPin, true);
            bool bBit = gpio_get(iSDCmdPin);
            sleep_us_high_power(SD_SLEEP);
            pvResponse[iByte] |= bBit;
        }
        iBit = 7;
    }
    return true;
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
        sleep_us_high_power(SD_SLEEP);
        gpio_put(iSDClkPin, true);
        sleep_us_high_power(SD_SLEEP);
    }
}


void sd_initial_tick(int iSDClkPin, int iSDCmdPin)
{
    gpio_set_dir(iSDCmdPin, GPIO_OUT);
    gpio_put(iSDCmdPin, true);

    for (int i = 0; i < 80; i++)
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
    for (int i = 0; i < 1000; i++)
    {
        gpio_put(iSDClkPin, false);
        sleep_us_high_power(SD_SLEEP);
        gpio_put(iSDClkPin, true);
        bool bBit = gpio_get(iSDCmdPin);
        sleep_us_high_power(SD_SLEEP);
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
            sd_clock_tick(iSDClkPin, iSDCmdPin, 4, GPIO_OUT);
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

#pragma pack (push, 1)
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
#pragma pack (pop)


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
    bool bDonePoweringOn;
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
        pOCR->bDonePoweringOn = (pResponse->uiOCR3 & 0x80) == 0x80;
        if (pOCR->bDonePoweringOn)
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
        }
        return true;
    }
    return false;
}


bool repeat_sd_acmd41_application_operating_condition(int iSDClkPin, int iSDCmdPin, u_int16_t uiRCA, SSDOCR* pOCR)
{
    bool bResult;

    for (int iCount = 0; iCount < 1000; iCount++)
    {
        bResult = sd_acmd41_application_operating_condition(iSDClkPin, iSDCmdPin, 0, pOCR);
        if (bResult)
        {
            if (pOCR->bDonePoweringOn)
            {
                return true;
            }
        }
        else
        {
            return false;
        }
    }
    return false;
}


#pragma pack (push, 1)
struct SSDResponseR2CID
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
#pragma pack (pop)


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


bool sd_cmd2_send_cid(int iSDClkPin, int iSDCmdPin, SSDCID* pCID)
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
        SSDResponseR2CID* pResponse = (SSDResponseR2CID*)aResponse;
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


#pragma pack (push, 1)
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
#pragma pack (pop)


enum ESDState
{
    SDS_Idle,
    SDS_Ready,
    SDS_Identify,
    SDS_StandBy,
    SDS_Transmit,
    SDS_Data,
    SDS_Receive,
    SDs_Program,
    SDS_Disabled,

    SDS_Reserved
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


int address_argument(uint16_t uiAddress)
{
    int iArgument = uiAddress;
    iArgument <<= 16;

    return iArgument;
}


int calculate_nanosecond_multiplier(uint8_t uiValue)
{
    if (uiValue == 0)
    {
        return 1;  //nano
    }
    if (uiValue == 1)
    {
        return 10;
    }
    if (uiValue == 2)
    {
        return 100;
    }
    if (uiValue == 3)
    {
        return 1000;  //micro
    }
    if (uiValue == 4)
    {
        return 10000;
    }
    if (uiValue == 5)
    {
        return 100000;
    }
    if (uiValue == 6)
    {
        return 1000000;  //milli
    }
    if (uiValue == 7)
    {
        return 10000000;
    }
    return 0;
}


float calculate_digit_multiplier(uint8_t uiValue)
{
    if (uiValue == 1)
    {
        return 1.0f;
    }
    if (uiValue == 2)
    {
        return 1.2f;
    }
    if (uiValue == 3)
    {
        return 1.3f;
    }
    if (uiValue == 4)
    {
        return 1.5f;
    }
    if (uiValue == 5)
    {
        return 2.0f;
    }
    if (uiValue == 6)
    {
        return 2.5f;
    }
    if (uiValue == 7)
    {
        return 3.0f;
    }
    if (uiValue == 8)
    {
        return 3.5f;
    }
    if (uiValue == 9)
    {
        return 4.0f;
    }
    if (uiValue == 10)
    {
        return 4.5f;
    }
    if (uiValue == 11)
    {
        return 5.0f;
    }
    if (uiValue == 12)
    {
        return 5.5f;
    }
    if (uiValue == 13)
    {
        return 6.0f;
    }
    if (uiValue == 14)
    {
        return 7.0f;
    }
    if (uiValue == 15)
    {
        return 8.0f;
    }
    return -1.0f;
}


int calculate_transmission_speed_multiplier(uint8_t uiValue)
{
    if (uiValue == 0)
    {
        return 100'000;  //100 Kbits/second
    }
    if (uiValue == 1)
    {
        return 1'000'000;  //1 Mbits/second
    }
    if (uiValue == 2)
    {
        return 10'000'000;
    }
    if (uiValue == 3)
    {
        return 100'000'000;
    }
    return 0;
}


#pragma pack (push, 1)
struct SSDResponseR2CSD
{
    uint8_t     uiReserved; //Bit 7   : 0 (Start transmit)
                            //Bit 6   : 0 (Card transmit)
                            //Bit 5..0: 0b111111

    uint8_t     uiCSDStructure; //Bit 7..6 : 0b00 (CSD structure)                          
                                //Bit 5..0 : 0b000000

    uint8_t     uiDataReadAccessTime1;
    uint8_t     uiDataReadAccessTime0;  //In clock ticks
    uint8_t     uiMaxDataTransferRate;  //0x32 or 0x5a
    uint8_t     uiCardCommandClasses1;  //          0b01X11011
    uint8_t     uiMaxReadBlockLength;   //Bit 7..4: 0b0101
                                        //Bit 3..0: 0xX (Length)
                                        
    uint8_t     uiFlags4;   //Bit 7    : Read partial block
                            //Bit 6    : Write misaligned block
                            //Bit 5    : Read misaligned block
                            //Bit 4    : DSR implemented
                            //Bit 3..2 : Reserved
                            //Bit 1..0 : Device size [11..10]

    uint8_t     uiDeviceSize;  // Device size [9..2]

    uint8_t     uiCurrentRatings1;  //Bit 7..6  : Device size [1..0]
                                    //Bit 5..3  : Max read current at VDD_min
                                    //Bit 2..0  : Max read current at VDD_max
    uint8_t     uiCurrentRatings0;  //Bit 7..5  : Max write current at VDD_min
                                    //Bit 4..2  : Max write current at VDD_max
                                    //Bit 1..0  : Device size multiplier [2..1]

    uint8_t     uiFlags3;   //Bit 7    : Device size multiplier [0]
                            //Bit 6    : Erase single block
                            //Bit 5..0 : Erase sector size [5..1]

    uint8_t     uiFlags2;   //Bit 7    : Erase sector size [0]
                            //Bit 6..0 : Write protect group size

    uint8_t     uiFlags1;   //Bit 7    : Write protect group
                            //Bit 6..5 : 0b00
                            //Bit 4..2 : Write speed factor
                            //Bit 1..0 : Max write data block length [3..2]

    uint8_t     uiWriteBlock;   //Bit 7..6 : Max write block length [1..0]
                                //Bit 5    : Write block partial
                                //Bit 4..0 : 0b00000

    uint8_t     uiFlags0;   //Bit 7    : File format group
                            //Bit 6    : Copy
                            //Bit 5    : Permanent write protection                            
                            //Bit 4    : Temporary write protection
                            //Bit 3..2 : File format
                            //Bit 1..0 : 0b00

    uint8_t uiCrc7;  //Bit 6..1: CRC7
                     //Bit 0   : 1 (Stop transmit)
};
#pragma pack (pop)


enum ESDRegisterStructure
{
    SDRS_StandardCapacity,
    SDRS_HighCapacity,
    SDRS_UltraCapacity,

    SDRS_Reserved
};


enum ESDFileFormat
{
    SDFF_HardDiskLikeWithPartitionTable,
    SDFF_FloppyDiskLikeBootSectorOnly,
    SDFF_UniversalFileFormat,
    SDFF_Unknown,

    SDFF_Reserved
};

struct SSDCSD
{
    ESDRegisterStructure    eCSDStructure;
    float                   fAsynchronousDataAccessTime;  //Nanoseconds.
    float                   fWorstCaseDataAccessTime;  //Clock cycles.
    float                   fSingleDataMaxTransmissionSpeed;  //Bits per second.
    bool                    bCommandClassSwitch;
    bool                    bCommandClassWriteProtect;
    bool                    bCommandClassLockCard;
    bool                    bCommandClassExtensions;
    int                     iMaxReadBlockLength;
    bool                    bReadPartialBlock;
    bool                    bWriteMisalignedBlock;
    bool                    bReadMisalignedBlock;
    bool                    bDSRImplemented;
    int                     iDeviceSize;
    uint8_t                 uiWriteSpeedFactor;
    int                     iMaxWriteBlocklength;
    bool                    bWritePartialBlock;
    bool                    bCopy;
    bool                    bPermanentWriteProtection;
    bool                    bTemporaryWriteProtection;
    ESDFileFormat           eFileFormat;
};


bool sd_cmd9_send_csd(int iSDClkPin, int iSDCmdPin, uint16_t uiAddress, SSDCSD* pCSD)
{
    uint8_t aCommand[6];
    uint8_t aResponse[17];
    bool bResult;

    build_command(aCommand, 9, address_argument(uiAddress));
    send_command(aCommand, iSDClkPin, iSDCmdPin);
    sd_clock_tick(iSDClkPin, iSDCmdPin, 4, GPIO_IN);

    bResult = receive_response(iSDClkPin, iSDCmdPin, 17, aResponse);
    if (bResult)
    {
        SSDResponseR2CSD* pResponse = (SSDResponseR2CSD*)aResponse;
        if (pResponse->uiReserved == 0x3F)
        {
            uint8_t uiExpectedCRC = crc7(&aResponse[1], 15);
            uiExpectedCRC <<= 1;
            uiExpectedCRC |= 1;
            if (pResponse->uiCrc7 == uiExpectedCRC)
            {
                pCSD->eCSDStructure = (ESDRegisterStructure)(pResponse->uiCSDStructure >> 6);

                int iMultiplier = calculate_nanosecond_multiplier(pResponse->uiDataReadAccessTime1 & 0x7);
                float fDigit = calculate_digit_multiplier((pResponse->uiDataReadAccessTime1 >> 3) & 0xF);
                pCSD->fAsynchronousDataAccessTime = iMultiplier * fDigit;

                iMultiplier = calculate_nanosecond_multiplier(pResponse->uiDataReadAccessTime0 & 0x7);
                fDigit = calculate_digit_multiplier((pResponse->uiDataReadAccessTime0 >> 3) & 0xF);
                pCSD->fWorstCaseDataAccessTime = iMultiplier * fDigit;

                iMultiplier = calculate_transmission_speed_multiplier(pResponse->uiMaxDataTransferRate & 0x7);
                fDigit = calculate_digit_multiplier((pResponse->uiMaxDataTransferRate >> 3) & 0xF);
                pCSD->fSingleDataMaxTransmissionSpeed = iMultiplier * fDigit;

                uint16_t uiCardClasses = (pResponse->uiCardCommandClasses1 << 4) | (pResponse->uiMaxReadBlockLength >> 4);

                pCSD->bCommandClassSwitch = (uiCardClasses & (1 << 10)) == 1 << 10;
                pCSD->bCommandClassWriteProtect = (uiCardClasses & (1 << 6)) == 1 << 6;
                pCSD->bCommandClassLockCard = (uiCardClasses & (1 << 7)) == 1 << 7;
                pCSD->bCommandClassExtensions = (uiCardClasses & (1 << 11)) == 1 << 11;

                pCSD->iMaxReadBlockLength = 1 << (pResponse->uiMaxReadBlockLength & 0xF);

                pCSD->bReadPartialBlock = (pResponse->uiFlags4 & 0x80) == 0x80;
                pCSD->bWriteMisalignedBlock = (pResponse->uiFlags4 & 0x40) == 0x40;
                pCSD->bReadMisalignedBlock = (pResponse->uiFlags4 & 0x20) == 0x20;
                pCSD->bDSRImplemented = (pResponse->uiFlags4 & 0x10) == 0x10;

                int iCSize = (pResponse->uiFlags4 & 0x3) << 8;
                iCSize |= pResponse->uiDeviceSize;
                iCSize <<= 2;
                iCSize |= pResponse->uiCurrentRatings1 >> 6;

                int iCSizeMultiplier = pResponse->uiCurrentRatings0 & 0x3;
                iCSizeMultiplier <<= 1;
                iCSizeMultiplier |= (pResponse->uiFlags3 & 0x80) >> 7;
                iMultiplier = 1 << (iCSizeMultiplier+2);
                int iBlockNr = (iCSize + 1) * iMultiplier;

                pCSD->iDeviceSize = pCSD->iMaxReadBlockLength * iBlockNr;

                pCSD->uiWriteSpeedFactor = 1 << ((pResponse->uiFlags1 >> 2) & 0x7);

                int iMaxWriteBlocklength = (pResponse->uiFlags1 & 0x3) << 2;
                iMaxWriteBlocklength |= (pResponse->uiWriteBlock >> 6) & 0x3;
                pCSD->iMaxWriteBlocklength = 1 << iMaxWriteBlocklength;

                pCSD->bWritePartialBlock = (pResponse->uiWriteBlock & 0x20) == 0x20;

                bool bFileFormatGroup = (pResponse->uiFlags0 & 0x80) == 0x80;
                pCSD->bCopy = (pResponse->uiFlags0 & 0x40) == 0x40;
                pCSD->bPermanentWriteProtection = (pResponse->uiFlags0 & 0x20) == 0x20;
                pCSD->bTemporaryWriteProtection = (pResponse->uiFlags0 & 0x10) == 0x10;

                if (bFileFormatGroup)
                {
                    pCSD->eFileFormat = SDFF_Reserved;
                }
                else
                {
                    pCSD->eFileFormat = (ESDFileFormat)((pResponse->uiFlags0 >> 2) & 0x3);
                }

                return true;
            }
        }
    }
    return false;
}


struct SDCardStatus
{
    bool        bOutOfRange;
    bool        bAddressError;
    bool        bBlockLengthError;
    bool        bSequenceEraseError;
    bool        bWriteBlockEraseError;
    bool        bWriteProtectedViolation;
    bool        bCardIsLocked;
    bool        bLockOrUnlockFailed;
    bool        bPreviousCRCFailed;
    bool        bIllegalCommand;
    bool        bInternalECCFailed;
    bool        bInternalError;
    bool        bGeneralError;

    bool        bCSDOverwriteError;
    bool        bErasePartiallyFailed;
    bool        bInternalECCDisabled;
    bool        bEraseNotExecuted;

    ESDState    eState;

    bool        bReadyForData;
    bool        bExtensionFunctionEvent;
    bool        bApplicationCommandExpected;
    bool        bAuthenticationError;
};


bool sd_cmd7_select_or_deselect_card(int iSDClkPin, int iSDCmdPin, uint16_t uiAddress, SDCardStatus* pStatus)
{
    uint8_t aCommand[6];
    uint8_t aResponse[6];
    bool    bResult;

    build_command(aCommand, 7, address_argument(uiAddress));
    send_command(aCommand, iSDClkPin, iSDCmdPin);
    sd_clock_tick(iSDClkPin, iSDCmdPin, 4, GPIO_IN);

    bResult = receive_response(iSDClkPin, iSDCmdPin, 6, aResponse);
    if (bResult)
    {
        SSDResponseR1* pResponse = (SSDResponseR1*)aResponse;
        if (pResponse->uiCmd == 7)
        {
            uint8_t uiExpectedCRC = crc7(aResponse, 5);
            uiExpectedCRC <<= 1;
            uiExpectedCRC |= 1;
            if (pResponse->uiCrc7 == uiExpectedCRC)
            {
                pStatus->bOutOfRange = (pResponse->uiStatus3 & 0x80) == 0x80;
                pStatus->bAddressError = (pResponse->uiStatus3 & 0x40) == 0x40;
                pStatus->bBlockLengthError = (pResponse->uiStatus3 & 0x20) == 0x20;
                pStatus->bSequenceEraseError = (pResponse->uiStatus3 & 0x10) == 0x10;
                pStatus->bWriteBlockEraseError = (pResponse->uiStatus3 & 0x8) == 0x8;
                pStatus->bWriteProtectedViolation = (pResponse->uiStatus3 & 0x4) == 0x4;
                pStatus->bCardIsLocked = (pResponse->uiStatus3 & 0x2) == 0x2;
                pStatus->bLockOrUnlockFailed = (pResponse->uiStatus3 & 0x1) == 0x1;
                pStatus->bPreviousCRCFailed = (pResponse->uiStatus2 & 0x80) == 0x80;
                pStatus->bIllegalCommand = (pResponse->uiStatus2 & 0x40) == 0x40;
                pStatus->bInternalECCFailed = (pResponse->uiStatus2 & 0x20) == 0x20;
                pStatus->bInternalError = (pResponse->uiStatus2 & 0x10) == 0x10;
                pStatus->bGeneralError = (pResponse->uiStatus2 & 0x8) == 0x8;

                pStatus->bCSDOverwriteError = (pResponse->uiStatus2 & 0x1) == 0x1;
                pStatus->bErasePartiallyFailed = (pResponse->uiStatus1 & 0x80) == 0x80;
                pStatus->bInternalECCDisabled = (pResponse->uiStatus1 & 0x40) == 0x40;
                pStatus->bEraseNotExecuted = (pResponse->uiStatus1 & 0x20) == 0x20;

                pStatus->eState = (ESDState)((pResponse->uiStatus1 & 0x1E) >> 1);

                pStatus->bReadyForData = (pResponse->uiStatus1 & 0x1) == 0x1;
                pStatus->bExtensionFunctionEvent = (pResponse->uiStatus0 & 0x40) == 0x40;
                pStatus->bApplicationCommandExpected = (pResponse->uiStatus0 & 0x20) == 0x20;
                pStatus->bAuthenticationError = (pResponse->uiStatus0 & 0x8) == 0x8;

                return true;
            }
        }
    }
    return false;
}


bool read_data(int iSDClkPin, int iSDDat0Pin, int iExpectedBytes, uint8_t* pvData)
{
    for (int iByte = 0; iByte < iExpectedBytes; iByte++)
    {
        pvData[iByte] = 0;
        for (int iBit = 7; iBit >= 0; iBit--)
        {
            gpio_put(iSDClkPin, false);
            gpio_put(iSDClkPin, false);

            pvData[iByte] <<= 1;

            gpio_put(iSDClkPin, true);
            gpio_put(iSDClkPin, true);

            bool bBit = gpio_get(iSDDat0Pin);
            pvData[iByte] |= bBit;
        }
    }
    return true;
}


bool receive_data(int iSDClkPin, int iSDCmdPin, int iSDDat0Pin, int iExpectedBytes, uint8_t* pvData)
{
    if (gpio_get_dir(iSDDat0Pin) == GPIO_OUT)
    {
        gpio_set_dir(iSDDat0Pin, GPIO_IN);
    }

    bool  bCardTransmit = false;
    for (int i = 0; i < 2000; i++)
    {
        gpio_put(iSDClkPin, false);
        sleep_us_high_power(SD_SLEEP);
        gpio_put(iSDClkPin, true);
        bool bBit = gpio_get(iSDDat0Pin);
        sleep_us_high_power(SD_SLEEP);
        if (!bBit)
        {
            bCardTransmit = true;
            break;
        }
    }

    if (bCardTransmit)
    {
        bool bValidResponse = read_data(iSDClkPin, iSDDat0Pin, iExpectedBytes, pvData);
        if (bValidResponse)
        {
            uint16_t    uiCRC16Read;
            uint8_t     aData[2];
            uint16_t    uiExpectedCRC16;
            bValidResponse = read_data(iSDClkPin, iSDDat0Pin, 2, aData);
            sd_clock_tick(iSDClkPin, iSDCmdPin, 4, GPIO_OUT);
            uiCRC16Read = (aData[0] << 8) | aData[1];
            if (bValidResponse)
            {
                uiExpectedCRC16 = crc16(pvData, iExpectedBytes);
                if (uiExpectedCRC16 == uiCRC16Read)
                {
                    return true;
                }

            }
        }
    }
    return false;
}


bool sd_cmd17_read_single_block(int iSDClkPin, int iSDCmdPin, int iSDDat0Pin, int iBlock, int iExpectedBytes, uint8_t* pvData)
{
    uint8_t aCommand[6];
    uint8_t aResponse[6];
    bool    bResult;

    build_command(aCommand, 17, iBlock);
    send_command(aCommand, iSDClkPin, iSDCmdPin);
    sd_clock_tick(iSDClkPin, iSDCmdPin, 4, GPIO_IN);

    bResult = receive_response(iSDClkPin, iSDCmdPin, 6, aResponse);
    if (bResult)
    {
        SSDResponseR1* pResponse = (SSDResponseR1*)aResponse;
        if (pResponse->uiCmd == 17)
        {
            uint8_t uiExpectedCRC = crc7(aResponse, 5);
            uiExpectedCRC <<= 1;
            uiExpectedCRC |= 1;
            if (pResponse->uiCrc7 == uiExpectedCRC)
            {
                receive_data(iSDClkPin, iSDCmdPin, iSDDat0Pin, iExpectedBytes, pvData);
                return true;
            }
        }
    }
    return false;
}


struct SSDFunctionSwitchStatus
{
    uint16_t    uiMaximumCurrentInMilliAmps;
    uint16_t    uiFunctionGroup6SupportBits;
    uint16_t    uiFunctionGroup5SupportBits;
    uint16_t    uiFunctionGroup4SupportBits;
    uint16_t    uiFunctionGroup3SupportBits;
    uint16_t    uiFunctionGroup2SupportBits;
    uint16_t    uiFunctionGroup1SupportBits;

    uint8_t     uiFunctionGroup6Selection;
    uint8_t     uiFunctionGroup5Selection;
    uint8_t     uiFunctionGroup4Selection;
    uint8_t     uiFunctionGroup3Selection;
    uint8_t     uiFunctionGroup2Selection;
    uint8_t     uiFunctionGroup1Selection;

    uint8_t     uiDataStructureVersion;

    uint16_t    uiFunctionGroup6BusyStatus;
    uint16_t    uiFunctionGroup5BusyStatus;
    uint16_t    uiFunctionGroup4BusyStatus;
    uint16_t    uiFunctionGroup3BusyStatus;
    uint16_t    uiFunctionGroup2BusyStatus;
    uint16_t    uiFunctionGroup1BusyStatus;
};


bool sd_cmd6_switch(int iSDClkPin, int iSDCmdPin, int iSDDat0Pin, bool bSwitch, uint8_t uiPowerLimit, uint8_t uiDriveStrength, uint8_t uiCommandSystem, uint8_t uiAccessMode, SSDFunctionSwitchStatus* pFunctionSwitchStatus)
{
    uint8_t     aCommand[6];
    uint8_t     aResponse[6];
    bool        bResult;
    uint8_t     aDataResponse[512];
    uint32_t    uiArgument;

    uiArgument = bSwitch ? 0x80000000 : 0;
    build_command(aCommand, 6, uiArgument | (uiPowerLimit << 12) | (uiDriveStrength << 8) | (uiCommandSystem << 4) | uiAccessMode);
    send_command(aCommand, iSDClkPin, iSDCmdPin);
    sd_clock_tick(iSDClkPin, iSDCmdPin, 4, GPIO_IN);

    bResult = receive_response(iSDClkPin, iSDCmdPin, 6, aResponse);
    if (bResult)
    {
        SSDResponseR1* pResponse = (SSDResponseR1*)aResponse;
        if (pResponse->uiCmd == 6)
        {
            uint8_t uiExpectedCRC = crc7(aResponse, 5);
            uiExpectedCRC <<= 1;
            uiExpectedCRC |= 1;
            if (pResponse->uiCrc7 == uiExpectedCRC)
            {
                memset(aDataResponse, 0, 512);
                bResult = receive_data(iSDClkPin, iSDCmdPin, iSDDat0Pin, 512, aDataResponse);
                {
                    pFunctionSwitchStatus->uiMaximumCurrentInMilliAmps = aDataResponse[0] << 8 | aDataResponse[1];

                    pFunctionSwitchStatus->uiFunctionGroup6SupportBits = aDataResponse[2] << 8 | aDataResponse[3];
                    pFunctionSwitchStatus->uiFunctionGroup5SupportBits = aDataResponse[4] << 8 | aDataResponse[5];
                    pFunctionSwitchStatus->uiFunctionGroup4SupportBits = aDataResponse[6] << 8 | aDataResponse[7];
                    pFunctionSwitchStatus->uiFunctionGroup3SupportBits = aDataResponse[8] << 8 | aDataResponse[9];
                    pFunctionSwitchStatus->uiFunctionGroup2SupportBits = aDataResponse[10] << 8 | aDataResponse[11];
                    pFunctionSwitchStatus->uiFunctionGroup1SupportBits = aDataResponse[12] << 8 | aDataResponse[13];

                    pFunctionSwitchStatus->uiFunctionGroup6Selection = aDataResponse[14] >> 4;
                    pFunctionSwitchStatus->uiFunctionGroup5Selection = aDataResponse[14] & 0xF;
                    pFunctionSwitchStatus->uiFunctionGroup4Selection = aDataResponse[15] >> 4;
                    pFunctionSwitchStatus->uiFunctionGroup3Selection = aDataResponse[15] & 0xF;
                    pFunctionSwitchStatus->uiFunctionGroup2Selection = aDataResponse[16] >> 4;
                    pFunctionSwitchStatus->uiFunctionGroup1Selection = aDataResponse[16] & 0xF;

                    pFunctionSwitchStatus->uiDataStructureVersion = aDataResponse[17];

                    pFunctionSwitchStatus->uiFunctionGroup6BusyStatus = aDataResponse[18] << 8 | aDataResponse[19];
                    pFunctionSwitchStatus->uiFunctionGroup5BusyStatus = aDataResponse[20] << 8 | aDataResponse[21];
                    pFunctionSwitchStatus->uiFunctionGroup4BusyStatus = aDataResponse[22] << 8 | aDataResponse[23];
                    pFunctionSwitchStatus->uiFunctionGroup3BusyStatus = aDataResponse[24] << 8 | aDataResponse[25];
                    pFunctionSwitchStatus->uiFunctionGroup2BusyStatus = aDataResponse[26] << 8 | aDataResponse[27];
                    pFunctionSwitchStatus->uiFunctionGroup1BusyStatus = aDataResponse[28] << 8 | aDataResponse[29];
                    
                    return true;
                }
            }
        }
    }
    return false;
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


    bool                        bResult;
    SSDOCR                      sOCR;
    SSDCID                      sCID;
    SSDCSD                      sCSD;
    SSDR6Status                 sR6Status;
    uint16_t                    uiAddress;
    SDCardStatus                sStatus;
    SSDFunctionSwitchStatus     sSwitchStatus;
    uint8_t                     aData[512];
    memset(aData, 0xff, 512);

    sd_initial_tick(iSDClkPin, iSDCmdPin);

    sd_cmd0_go_idle(iSDClkPin, iSDCmdPin, iSDDat3Pin, false);

    bResult = sd_cmd8_interface_condition(iSDClkPin, iSDCmdPin);
    if (bResult)
    {
        bResult = repeat_sd_acmd41_application_operating_condition(iSDClkPin, iSDCmdPin, 0, &sOCR);
        if (bResult)
        {
            bResult = sd_cmd2_send_cid(iSDClkPin, iSDCmdPin, &sCID);
            if (bResult)
            {
                bResult = sd_cmd3_publish_relative_address(iSDClkPin, iSDCmdPin, &uiAddress, &sR6Status);
                if (bResult)
                {
                    bResult = sd_cmd9_send_csd(iSDClkPin, iSDCmdPin, uiAddress, &sCSD);
                    if (bResult)
                    {
                        if (sCSD.iMaxReadBlockLength == 512)
                        {
                            bResult = sd_cmd7_select_or_deselect_card(iSDClkPin, iSDCmdPin, uiAddress, &sStatus);
                            if (bResult)
                            {
                                if (sCSD.bCommandClassSwitch)
                                {
                                    bResult = sd_cmd6_switch(iSDClkPin, iSDCmdPin, iSDDat0Pin, true, 0xF, 0xF, 0xF, 0x1, &sSwitchStatus);
                                    if (bResult)
                                    {
                                        bResult = sd_cmd6_switch(iSDClkPin, iSDCmdPin, iSDDat0Pin, false, 0xF, 0xF, 0xF, 0x1, &sSwitchStatus);
                                        if (bResult)
                                        {
                                            if (sSwitchStatus.uiFunctionGroup1Selection == 0x1)
                                            {
                                                bResult = sd_cmd17_read_single_block(iSDClkPin, iSDCmdPin, iSDDat0Pin, 41024, sCSD.iMaxReadBlockLength, aData);
                                                if  (bResult)
                                                {
                                                    int iCmp = memcmp(aData, "John", 4);
                                                    if (iCmp == 0)
                                                    {
                                                        blink_led(25'000);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

                                //ACMD6 - Wide bus mode.
                            }
                        }
                    }
                }
            }
        }
    }
    blink_led(200'000);







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


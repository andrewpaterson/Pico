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

bool receive_response(int iSDClkPin, int iSDCmdPin, uint8_t* pvResponse)
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
        return read_response(iSDClkPin, iSDCmdPin, 6, pvResponse);
    }
    else
    {
        return false;
    }
}

void sd_clock_tick(int iSDClkPin, int iSDCmdPin, int iFullCycles)
{
    if (gpio_get_dir(iSDCmdPin) == GPIO_IN)
    {
        gpio_set_dir(iSDCmdPin, GPIO_OUT);
    }

    gpio_put(iSDCmdPin, true);

    for (int i = 0; i < iFullCycles; i++)
    {
        gpio_put(iSDClkPin, false);
        sleep_us_high_power(5);
        gpio_put(iSDClkPin, true);
        sleep_us_high_power(5);
    }
}

bool sd_go_idle(int iSDClkPin, int iSDCmdPin, int iSDDat3Pin, bool bSDMode)
{
    uint8_t aCommand[6];

    build_command(aCommand, 0, 0);
    
    gpio_set_dir(iSDDat3Pin, GPIO_OUT);
    gpio_put(iSDDat3Pin, !bSDMode);

    sd_clock_tick(iSDClkPin, iSDCmdPin, 8);

    send_command(aCommand, iSDClkPin, iSDCmdPin);

    gpio_set_dir(iSDDat3Pin, GPIO_IN);

    sd_clock_tick(iSDClkPin, iSDCmdPin, 8);

    return true;
}

bool sd_interface_condition(int iSDClkPin, int iSDCmdPin)
{
    uint8_t aCommand[6];
    uint8_t aResponse[6];
    bool bResult;

    build_command(aCommand, 8, 0x000001AA);
    send_command(aCommand, iSDClkPin, iSDCmdPin);

    bResult = receive_response(iSDClkPin, iSDCmdPin, aResponse);
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

    sleep_us_high_power(400000);

    sd_go_idle(iSDClkPin, iSDCmdPin, iSDDat3Pin, false);
    sd_interface_condition(iSDClkPin, iSDCmdPin);















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


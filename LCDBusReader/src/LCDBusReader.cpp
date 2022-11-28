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
#include "SDCard.h"
#include "SDCardTest.h"
#include "HexToMem.h"


int     giDelay;


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


void do_shift_LCD(void)
{
    S595OutPins sPins;
    sPins.Init(PIN_NOT_SET, 22, 21, 20, PIN_NOT_SET);
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


void do_parallel_LCD(void)
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


void block_reads(SSDCardPins* pPins, uint16_t uiAddress)
{
    uint8_t         aData[512];
    bool            bResult;
    SSDCardStatus   sStatus;
    uint8_t         aMultiData[512 * 7];
    uint8_t         aMultiDataExpected[512 * 7];

    memset(aData, 0xff, 512);
    bResult = sd_cmd17_read_single_block_narrow(pPins, 41024, 512, aData, &sStatus);
    if (bResult)
    {
        int iCmp = memcmp(aData, "John West wrote:", 16);
        if (iCmp == 0)
        {
            bResult = sd_acmd6_set_bus_width(pPins, uiAddress, 2, &sStatus);
            if (bResult)
            {
                memset(aData, 0xff, 512);
                bResult = sd_cmd17_read_single_block_wide(pPins, 41024, 512, aData, &sStatus);
                if (bResult)
                {
                    int iCmp = memcmp(aData, "John West wrote:", 16);
                    if (iCmp == 0)
                    {
                        memset(aMultiDataExpected, 0xFF, 512 * 7);
                        read_hex_string_into_memory(aMultiDataExpected, 512 * 7, gszSDTestExpectedData);
                        
                        bResult = sd_cmd_23_set_block_count(pPins, 7, &sStatus);
                        if (bResult)
                        {
                            memset(aMultiData, 0xCC, 512 * 7);
                            bResult = sd_cmd18_read_multiple_blocks_wide(pPins, 41024, 7, 512, aMultiData, &sStatus);
                            if (bResult)
                            {
                                int iCmp = memcmp(aMultiData, aMultiDataExpected, 512 * 7);
                               if (iCmp == 0)
                                {
                                    blink_led(25'000);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    blink_led(200'000);
}

void do_sd_card()
{

    SSDCardPins sPins;

    sPins.Init(6, 7, 2 ,3 ,4 ,5);
    sd_card_init(&sPins);
    int iSDDat0Pin = 2;
    int iSDDat1Pin = 3;
    int iSDDat2Pin = 4;
    int iSDDat3Pin = 5;

    int iSDClkPin = 6;
    int iSDCmdPin = 7;

    bool                        bResult;
    SSDOCR                      sOCR;
    SSDCID                      sCID;
    SSDCSD                      sCSD;
    SSDR6Status                 sR6Status;
    uint16_t                    uiAddress;
    SSDCardStatus               sStatus;
    SSDFunctionSwitchStatus     sSwitchStatus;

    sd_initial_tick(&sPins);  //A slow 100Khz 80 count tick is important if only one go_idle command is sent.  400Khz is fine if the entire initialistion is restarted.

    for (int iRestartCount = 1; iRestartCount++;)  //Restarting is necessary only if cmd6 is issued (change power output and maximum clock speed).  This repeats up to 6 times.
    {
        sd_cmd0_go_idle(&sPins, false);

        bResult = sd_cmd8_interface_condition(&sPins);
        if (bResult)
        {
            bResult = repeat_sd_acmd41_application_operating_condition(&sPins, 0, &sOCR);  //This has repeated up to 260 times before a 'ready' response is returned.
            if (bResult)
            {
                bResult = sd_cmd2_send_cid(&sPins, &sCID);
                if (bResult)
                {
                    bResult = sd_cmd3_publish_relative_address(&sPins, &uiAddress, &sR6Status);
                    if (bResult)
                    {
                        bResult = sd_cmd9_send_csd(&sPins, uiAddress, &sCSD);
                        if (bResult)
                        {
                            if (sCSD.iMaxReadBlockLength == 512)
                            {
                                bResult = sd_cmd7_select_or_deselect_card(&sPins, uiAddress, &sStatus);
                                if (bResult)
                                {
                                    if (sCSD.bCommandClassSwitch)
                                    {
                                        bResult = sd_cmd6_switch(&sPins, true, 0x1, 0xF, 0xF, 0x1, &sStatus, &sSwitchStatus);
                                        if (bResult)
                                        {
                                            bResult = check_sd_cmd6_switch(&sPins, 0xF, 0xF, 0xF, 0x1, &sStatus, &sSwitchStatus);
                                            if (bResult)
                                            {
                                                block_reads(&sPins, uiAddress);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


void do_uart(void)
{
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


void init_io_and_led(void)
{
    stdio_init_all();
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
}


int main() 
{
    init_io_and_led();

    do_shift_LCD();
}


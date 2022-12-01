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
#include "W65C816Bus.h"


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


void do_shift_LCD(uint uiShiftPin, uint uiStorageLatchPin, uint uiDataOutPin)
{
    S595OutPins sPins;
    sPins.Init(PIN_NOT_SET, uiShiftPin, uiStorageLatchPin, uiDataOutPin, PIN_NOT_SET);
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


void do_ltc6903(int pinClk, int pinTx, int pinRx, int pinEnable, int iHertz)
{
    SSPIPins    sPins;

    sPins.Init(pinClk, pinTx, pinRx, pinEnable, false);
    
    init_spi(&sPins);
    put_LTC6903_frequency(&sPins, iHertz);
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



uart_inst_t* init_uart_inst(int iTxPin, int iRxPin, int iBaudRate)
{
    uart_inst_t* pUart = init_uart(iTxPin, iRxPin, iBaudRate);
    int iIRQ = get_uart_irq(pUart);

    irq_set_exclusive_handler(iIRQ, uart_receive_irq);
    irq_set_enabled(iIRQ, true);
    uart_set_irq_enables(pUart, true, false);

    return pUart;
}


void do_uart_master(int iTxPin, int iRxPin, int iBaudRate)
{
    uart_inst_t* pUart = init_uart_inst(iTxPin, iRxPin, iBaudRate);

    uart_puts(pUart, "Garbage\n");
    sleep_us_high_power(100'000);
    uart_puts(pUart, "Fast\n");
    sleep_us_high_power(1000'000);
    uart_puts(pUart, "Slow\n");
    sleep_us_high_power(100'000);
}

bool execute_message(SW65C816Pins* psPins, char* szMessage, int iData, int iAddress)
{
    if (memcmp(szMessage, "IO:Z", 4) == 0)
    {
        w65_disable_io(psPins);
    }
    else if (memcmp(szMessage, "A:", 2) == 0)
    {
        iAddress = strtol(&szMessage[2], NULL, 16);
        if (iAddress >= 0 && iAddress <= 0xFFFF)
        {
            write_uart_message(pUart, "OK");
        }
        else
        {
            write_uart_message(pUart, "B#");
        }
    }
    else if (memcmp(szMessage, "D:", 2) == 0)
    {
        iData = strtol(&szMessage[2], NULL, 16);
        if (iData >= 0 && iData <= 0xFF)
        {
            write_uart_message(pUart, "OK");
        }
        else
        {
            write_uart_message(pUart, "B#");
        }
    }
    else if (memcmp(szMessage, "IO:A", 4) == 0)
    {
        w65_address_out_data_in(psPins, iAddress);
        write_uart_message(pUart, "OK");
    }
    else if (memcmp(szMessage, "IO:D+A", 6) == 0)
    {
        w65_address_out_data_out(psPins, iData, iAddress);
        write_uart_message(pUart, "OK");
    }
    else if (memcmp(szMessage, "READ", 4) == 0)
    {
        iData = w65_read(psPins);
        if (iData < 0x10)
        {
            szMessage[0] = '0';
            itoa(iData, &szMessage[1], 16);
        }
        else
        {
            itoa(iData, szMessage, 16);
        }
        szMessage[2] = '\n';
        write_uart_message(pUart, szMessage);
    }
}


void do_uart_slave(int iTxPin, int iRxPin, int iBaudRate)
{
    SW65C816Pins    sPins;

    uart_inst_t* pUart = init_uart_inst(iTxPin, iRxPin, iBaudRate);

    sPins.Init( /* Data Pins */ 2, 3, 4, 5, 6, 7, 8, 9,
                /* Addr Pins */ 17, 16, 15, 14, 13, 12, 11, 10,
                                18, 19, 20, 21, 22, 26, 27, 28,
                                PIN_NOT_SET, PIN_NOT_SET, PIN_NOT_SET);
    w65_init(&sPins);

    bool    bLed = true;
    int     iCommandSpeed = 250000;
    int     iData = 0;
    int     iAddress = 0;
    char    szMessage[32];

    for (;;)
    {
        gpio_put(25, bLed);

        int delay = iCommandSpeed;

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
                execute_message(&sPins, szMessage, iData, iAddress);
            }
            sleep_us_high_power(10);
        }
        bLed = !bLed;
    }
}


void init_signal(int iPin, bool bValue)
{
    gpio_init(iPin);
    gpio_set_dir(iPin, GPIO_OUT);
    gpio_put(iPin, bValue);
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

    do_uart_slave(0, 1, 115200);

    int iResB = 8;
    int iNmiB = 9;
    int iIrqB = 10;
    int iAbortB = 11;
    int iBE = 12;

    init_signal(iBE, false);
    init_signal(iResB, false);
    init_signal(iNmiB, true);
    init_signal(iIrqB, true);
    init_signal(iAbortB, true);

    do_ltc6903(18, 19, 16, 17, 1'500);

    do_uart_master(0, 1, 115200);

    do_shift_LCD(22, 21, 20);
}


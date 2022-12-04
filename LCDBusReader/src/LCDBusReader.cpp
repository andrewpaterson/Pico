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
#include "Keypad.h"
#include "SDCardTest.h"
#include "HexToMem.h"
#include "W65C816Bus.h"
#include "W65C816Slave.h"
#include "W65C816Master.h"


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


// void do_shift_LCD(uint uiShiftPin, uint uiStorageLatchPin, uint uiDataOutPin)
// {
//     S595OutPins sPins;
//     sPins.Init(PIN_NOT_SET, uiShiftPin, uiStorageLatchPin, uiDataOutPin, PIN_NOT_SET);
//     init_shift(&sPins);

//     init_lcd(&sPins);
//     put_clear(&sPins);

//     int  i = 0;
//     char szLine1[17];
//     char szLine2[17];
//     while(true)
//     {
//         kitt(i, szLine1);
//         message(i, szLine2);

//         put_lines(&sPins, szLine1, szLine2);

//         sleep_us_high_power(200000);

//         i++;
//     }
// }


// void do_parallel_LCD(void)
// {
//     bool led = true;

//     S11BitLCDPins   sPins;

//     sPins.Init(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

//     init_lcd(&sPins);
//     put_clear(&sPins);

//     int  i = 0;
//     char szLine1[17];
//     char szLine2[17];
//     while(true)
//     {
//         kitt(i, szLine1);
//         message(i, szLine2);

//         put_lines(&sPins, szLine1, szLine2);

//         sleep_us_high_power(200000);

//         i++;
//     }
// }


void do_ltc6903(int pinClk, int pinTx, int pinRx, int pinEnable, int iHertz)
{
    SSPIPins    sPins;

    sPins.Init(pinClk, pinTx, pinRx, pinEnable, false);
    
    init_spi(&sPins);
    put_LTC6903_frequency(&sPins, iHertz);
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


void print_read_byte(S595OutPins* psLCDPins, int iData)
{
    char szLine1[17];

    if (iData >=0 && iData <= 0xF)
    {
        itoa(iData, &szLine1[1], 16);
        szLine1[0] = '0';
    }
    else if (iData > 0xF && iData <= 0xFF)
    {
        itoa(iData, szLine1, 16);
    }
    else
    {
        strcpy(szLine1, "Error");
    }    
    put_lines(psLCDPins, szLine1, (char*)"");
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


int change_frequency(SSPIPins* psPins, CW65C816Master* pcMaster, int iFrequency)
{
    pcMaster->BusEnable(false);
    put_LTC6903_frequency(psPins, iFrequency);
    pcMaster->FreeClock(true);
    pcMaster->BusEnable(true);
    return iFrequency;
}


void write_program(CW65C816Master* pcMaster)
{
    pcMaster->BusEnable(false);
    pcMaster->FreeClock(false);

    pcMaster->SramWriteEnable(true);

    pcMaster->Write(0xFFFC, 0x23, true);
    sleep_us_high_power(100000);
    pcMaster->Write(0xFFFD, 0x81, true);
    sleep_us_high_power(100000);
    pcMaster->Write(0x8123, 0x4C, true);
    sleep_us_high_power(100000);
    pcMaster->Write(0x8124, 0x23, true);
    sleep_us_high_power(100000);
    pcMaster->Write(0x8125, 0x81, true);
    sleep_us_high_power(100000);
    pcMaster->Write(0x8126, 0x00, true);
    sleep_us_high_power(100000);

    pcMaster->SramWriteEnable(false);

    pcMaster->HighZ();
}


int calculate_frequency(int iDelay)
{
    return (int)(1.0f / ((float)iDelay / 500000.0f));
}


void do_uart_master(int iTxPin, int iRxPin, int iBaudRate)
{
    SSPIPins    sLTCPins;
    int         iFrequency;

    sLTCPins.Init(18, 19, 16, 17, false);
    
    init_spi(&sLTCPins);
    iFrequency = 1500;
    put_LTC6903_frequency(&sLTCPins, iFrequency);

    S595OutPins sLCDPins;

    sLCDPins.Init(PIN_NOT_SET, 22, 21, 20, PIN_NOT_SET);
    init_shift(&sLCDPins);

    init_lcd(&sLCDPins);
    put_clear(&sLCDPins);

    S165InPins  sKeyPins;
    CKeypad     cKeypad;

    sKeyPins.Init(26, 27, 28, true, true, true);
    cKeypad.Init(&sKeyPins);
    
    uart_inst_t* pUart = init_uart_inst(iTxPin, iRxPin, iBaudRate);

    SW65C816Pins    sPins;
    CW65C816Master  cMaster;

    sPins.Init( /* Data Pins */ PIN_NOT_SET, PIN_NOT_SET, PIN_NOT_SET, PIN_NOT_SET, PIN_NOT_SET, PIN_NOT_SET, PIN_NOT_SET, PIN_NOT_SET,
                /* Addr Pins */ PIN_NOT_SET, PIN_NOT_SET, PIN_NOT_SET, PIN_NOT_SET, PIN_NOT_SET, PIN_NOT_SET, PIN_NOT_SET, PIN_NOT_SET,
                                PIN_NOT_SET, PIN_NOT_SET, PIN_NOT_SET, PIN_NOT_SET, PIN_NOT_SET, PIN_NOT_SET, PIN_NOT_SET, PIN_NOT_SET,
                                13, 14, 15);

    cMaster.Init(&sPins, pUart, 8, 9, 10, 11, 12, 4, 5, 16, 3);

    write_program(&cMaster);
    cMaster.Reset(true);
    cMaster.FreeClock(false);
    cMaster.BusEnable(true);

    int iState = 0;
    bool bLed = false;
    int  iTick = 0;

    uint64_t delay = 100'000;
    uint64_t start = time_us_64();
    uint64_t expectedEnd = start + delay;
    uint64_t end = start;
    iFrequency = calculate_frequency(delay);

    for (;;)
    {
        if (iTick == 4)
        {
            cMaster.Reset(false);
        }
        gpio_put(25, bLed);

        if (expectedEnd > end)
        {
            end = time_us_64();
        }
        else
        {
            if (iState == 0)
            {            
                bLed = !bLed;
                iTick++;
            }

            start = time_us_64();
            expectedEnd = start + delay;
            end = start;
        }

        if ((iState == 0) || (iState == 1))
        {            
            cMaster.Tick(bLed);
        }

        char szLine1[17];
        if (iFrequency > 0)
        {
            memset(szLine1, ' ', 16);
            szLine1[16] = '\0';
            sprintf(szLine1, "%i Hz", iFrequency);
            szLine1[strlen(szLine1)] = ' ';
        }
        else
        {
            strcpy(szLine1, "Half-Step       ");
        }
        put_lines(&sLCDPins, szLine1, "");

        cKeypad.Read();
        char c = cKeypad.GetPressed();

        if (c == '2')
        {
            cMaster.FreeClock(false);
            iState = 0;
            delay = 500'000;
            iFrequency = calculate_frequency(delay);
        }
        else if (c == '3')
        {
            cMaster.FreeClock(false);
            iState = 0;
            delay = 100'000;
            iFrequency = calculate_frequency(delay);
        }
        else if (c == '1')
        {
            cMaster.FreeClock(false);
            if (iState != 1)
            {
                iState = 1;
            }
            else
            {
                bLed = !bLed;
                iTick++;
            }
            iFrequency = 0;
        }
        else if (c == '*')
        {
            iTick = 0;
            cMaster.FreeClock(false);
            cMaster.Reset(true);
            iState  = 0;
            iFrequency = calculate_frequency(delay);
        }
        else if (c == '#')
        {
           cMaster.Reset(true);
           sleep_us_high_power(10000);
           cMaster.Reset(false);
        }
        else if (c == '0')
        {
            iState = 0;
            write_program(&cMaster);
            iFrequency = calculate_frequency(delay);
            cMaster.BusEnable(true);
        }
        else if (c == '4')
        {
            iFrequency = change_frequency(&sLTCPins, &cMaster, 1500);
            iState = 2;
        }
        else if (c == '5')
        {
            iFrequency = change_frequency(&sLTCPins, &cMaster, 500'000);
            iState = 2;
        }
        else if (c == '6')
        {
            iFrequency = change_frequency(&sLTCPins, &cMaster, 16'750'000);
            iState = 2;
        }
        else if (c == '7')
        {
            iFrequency = change_frequency(&sLTCPins, &cMaster, 33'470'000);
            iState = 2;
        }
        else if (c == '8')
        {
            iFrequency = change_frequency(&sLTCPins, &cMaster, 35'000'000);
            iState = 2;
        }
        else if (c == '9')
        {
            iFrequency = change_frequency(&sLTCPins, &cMaster, 42'000'000);
            iState = 2;
        }
        sleep_us_high_power(10000);

    }
}


void do_uart_slave(int iTxPin, int iRxPin, int iBaudRate)
{
    uart_inst_t* pUart = init_uart_inst(iTxPin, iRxPin, iBaudRate);

    SW65C816Pins    sPins;
    CW65C816Slave   cSlave;

    sPins.Init( /* Data Pins */ 2, 3, 4, 5, 6, 7, 8, 9,
                /* Addr Pins */ 17, 16, 15, 14, 13, 12, 11, 10,
                                18, 19, 20, 21, 22, 26, 27, 28,
                                PIN_NOT_SET, PIN_NOT_SET, PIN_NOT_SET);
    cSlave.Init(&sPins);

    bool    bLed = true;
    int     iCommandSpeed = 250000;
    char    szMessage[256];
    char    szResponse[256];
    bool    bResult;

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
            bool bNewMessage = read_uart_message(szMessage);
            if (bNewMessage)
            {
                bResult = cSlave.ExecuteMessage(szMessage, szResponse);
                write_uart_message(pUart, szResponse);
            }
            sleep_us_high_power(10);
        }
        bLed = !bLed;
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
    gpio_put(25, true);

    //do_uart_slave(0, 1, 115200);
    sleep_us_high_power(250'000);

    do_uart_master(0, 1, 115200);
    gpio_put(25, false);
}


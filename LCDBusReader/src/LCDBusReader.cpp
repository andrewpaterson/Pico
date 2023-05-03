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
#include "MAX5102.h"
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


void write(CW65C816Master* pcMaster, int iAddress, int iByte)
{
    pcMaster->Write(iAddress, iByte, true);
    sleep_us_high_power(5000);
}


void write_program(CW65C816Master* pcMaster)
{
    pcMaster->BusEnable(false);
    pcMaster->FreeClock(false);

    pcMaster->SramWriteEnable(true);

    write(pcMaster, 0x0000, 0x5B);
    write(pcMaster, 0x0001, 0x78);
    write(pcMaster, 0x0002, 0x78);
    write(pcMaster, 0x0003, 0x78);
    write(pcMaster, 0x0004, 0x78);
    write(pcMaster, 0x0005, 0x78);
    write(pcMaster, 0x0006, 0x78);
    write(pcMaster, 0x0007, 0x78);
    write(pcMaster, 0x0008, 0x78);
    write(pcMaster, 0x0009, 0x78);
    write(pcMaster, 0x000A, 0x78);
    write(pcMaster, 0x000B, 0x78);
    write(pcMaster, 0x000C, 0x78);
    write(pcMaster, 0x000D, 0x78);
    write(pcMaster, 0x000E, 0x78);
    write(pcMaster, 0x000F, 0x78);
    write(pcMaster, 0x0010, 0x78);
    write(pcMaster, 0x0011, 0x78);
    write(pcMaster, 0x0012, 0x78);
    write(pcMaster, 0x0013, 0x78);
    write(pcMaster, 0x0014, 0x78);
    write(pcMaster, 0x0015, 0x78);
    write(pcMaster, 0x0016, 0x5D);
    write(pcMaster, 0x0017, 0xFF);
    write(pcMaster, 0x0018, 0xFF);
    write(pcMaster, 0x4000, 0x58);
    write(pcMaster, 0x4001, 0xA9);
    write(pcMaster, 0x4002, 0x2E);
    write(pcMaster, 0x4003, 0x00);
    write(pcMaster, 0x4004, 0x20);
    write(pcMaster, 0x4005, 0x0E);
    write(pcMaster, 0x4006, 0x40);
    write(pcMaster, 0x4007, 0x00);
    write(pcMaster, 0x4008, 0x0C);
    write(pcMaster, 0x4009, 0x02);
    write(pcMaster, 0x400A, 0x22);
    write(pcMaster, 0x400B, 0x4C);
    write(pcMaster, 0x400C, 0x00);
    write(pcMaster, 0x400D, 0x40);
    write(pcMaster, 0x400E, 0xE2);
    write(pcMaster, 0x400F, 0x20);
    write(pcMaster, 0x4010, 0xA0);
    write(pcMaster, 0x4011, 0x01);
    write(pcMaster, 0x4012, 0x00);
    write(pcMaster, 0x4013, 0x99);
    write(pcMaster, 0x4014, 0x00);
    write(pcMaster, 0x4015, 0x00);
    write(pcMaster, 0x4016, 0xC8);
    write(pcMaster, 0x4017, 0xC0);
    write(pcMaster, 0x4018, 0x16);
    write(pcMaster, 0x4019, 0x00);
    write(pcMaster, 0x401A, 0xD0);
    write(pcMaster, 0x401B, 0xF7);
    write(pcMaster, 0x401C, 0x60);
    write(pcMaster, 0x401D, 0xE2);
    write(pcMaster, 0x401E, 0x20);
    write(pcMaster, 0x401F, 0xBD);
    write(pcMaster, 0x4020, 0x00);
    write(pcMaster, 0x4021, 0x00);
    write(pcMaster, 0x4022, 0xF0);
    write(pcMaster, 0x4023, 0x07);
    write(pcMaster, 0x4024, 0x99);
    write(pcMaster, 0x4025, 0x00);
    write(pcMaster, 0x4026, 0x00);
    write(pcMaster, 0x4027, 0xC8);
    write(pcMaster, 0x4028, 0xE8);
    write(pcMaster, 0x4029, 0x80);
    write(pcMaster, 0x402A, 0xF4);
    write(pcMaster, 0x402B, 0xC2);
    write(pcMaster, 0x402C, 0x20);
    write(pcMaster, 0x402D, 0x60);
    write(pcMaster, 0x402E, 0x49);
    write(pcMaster, 0x402F, 0x52);
    write(pcMaster, 0x4030, 0x51);
    write(pcMaster, 0x4031, 0x00);
    write(pcMaster, 0x4032, 0x4E);
    write(pcMaster, 0x4033, 0x4D);
    write(pcMaster, 0x4034, 0x49);
    write(pcMaster, 0x4035, 0x00);
    write(pcMaster, 0x4036, 0x41);
    write(pcMaster, 0x4037, 0x42);
    write(pcMaster, 0x4038, 0x4F);
    write(pcMaster, 0x4039, 0x52);
    write(pcMaster, 0x403A, 0x54);
    write(pcMaster, 0x403B, 0x00);
    write(pcMaster, 0x403C, 0x42);
    write(pcMaster, 0x403D, 0x52);
    write(pcMaster, 0x403E, 0x4B);
    write(pcMaster, 0x403F, 0x00);
    write(pcMaster, 0x4040, 0x43);
    write(pcMaster, 0x4041, 0x4F);
    write(pcMaster, 0x4042, 0x50);
    write(pcMaster, 0x4043, 0x00);
    write(pcMaster, 0x4044, 0x20);
    write(pcMaster, 0x4045, 0x2D);
    write(pcMaster, 0x4046, 0x20);
    write(pcMaster, 0x4047, 0x45);
    write(pcMaster, 0x4048, 0x6D);
    write(pcMaster, 0x4049, 0x75);
    write(pcMaster, 0x404A, 0x6C);
    write(pcMaster, 0x404B, 0x61);
    write(pcMaster, 0x404C, 0x74);
    write(pcMaster, 0x404D, 0x69);
    write(pcMaster, 0x404E, 0x6F);
    write(pcMaster, 0x404F, 0x6E);
    write(pcMaster, 0x4050, 0x20);
    write(pcMaster, 0x4051, 0x2D);
    write(pcMaster, 0x4052, 0x20);
    write(pcMaster, 0x4053, 0x20);
    write(pcMaster, 0x4054, 0x00);
    write(pcMaster, 0x4055, 0xFF);
    write(pcMaster, 0x4056, 0xFF);
    write(pcMaster, 0xE12B, 0x18);
    write(pcMaster, 0xE12C, 0xFB);
    write(pcMaster, 0xE12D, 0xC2);
    write(pcMaster, 0xE12E, 0x30);
    write(pcMaster, 0xE12F, 0xA9);
    write(pcMaster, 0xE130, 0xFF);
    write(pcMaster, 0xE131, 0x01);
    write(pcMaster, 0xE132, 0x1B);
    write(pcMaster, 0xE133, 0xE2);
    write(pcMaster, 0xE134, 0x20);
    write(pcMaster, 0xE135, 0xA9);
    write(pcMaster, 0xE136, 0x00);
    write(pcMaster, 0xE137, 0x48);
    write(pcMaster, 0xE138, 0xAB);
    write(pcMaster, 0xE139, 0xC2);
    write(pcMaster, 0xE13A, 0x20);
    write(pcMaster, 0xE13B, 0x5C);
    write(pcMaster, 0xE13C, 0x00);
    write(pcMaster, 0xE13D, 0x40);
    write(pcMaster, 0xE13E, 0x00);
    write(pcMaster, 0xE13F, 0x8B);
    write(pcMaster, 0xE140, 0x0B);
    write(pcMaster, 0xE141, 0xC2);
    write(pcMaster, 0xE142, 0x30);
    write(pcMaster, 0xE143, 0x48);
    write(pcMaster, 0xE144, 0xDA);
    write(pcMaster, 0xE145, 0x5A);
    write(pcMaster, 0xE146, 0xA2);
    write(pcMaster, 0xE147, 0x2E);
    write(pcMaster, 0xE148, 0x40);
    write(pcMaster, 0xE149, 0xA0);
    write(pcMaster, 0xE14A, 0x01);
    write(pcMaster, 0xE14B, 0x00);
    write(pcMaster, 0xE14C, 0x20);
    write(pcMaster, 0xE14D, 0x1D);
    write(pcMaster, 0xE14E, 0x40);
    write(pcMaster, 0xE14F, 0x80);
    write(pcMaster, 0xE150, 0x57);
    write(pcMaster, 0xE151, 0x8B);
    write(pcMaster, 0xE152, 0x0B);
    write(pcMaster, 0xE153, 0xC2);
    write(pcMaster, 0xE154, 0x30);
    write(pcMaster, 0xE155, 0x48);
    write(pcMaster, 0xE156, 0xDA);
    write(pcMaster, 0xE157, 0x5A);
    write(pcMaster, 0xE158, 0xA2);
    write(pcMaster, 0xE159, 0x32);
    write(pcMaster, 0xE15A, 0x40);
    write(pcMaster, 0xE15B, 0xA0);
    write(pcMaster, 0xE15C, 0x05);
    write(pcMaster, 0xE15D, 0x00);
    write(pcMaster, 0xE15E, 0x20);
    write(pcMaster, 0xE15F, 0x1D);
    write(pcMaster, 0xE160, 0x40);
    write(pcMaster, 0xE161, 0x80);
    write(pcMaster, 0xE162, 0x45);
    write(pcMaster, 0xE163, 0x8B);
    write(pcMaster, 0xE164, 0x0B);
    write(pcMaster, 0xE165, 0xC2);
    write(pcMaster, 0xE166, 0x30);
    write(pcMaster, 0xE167, 0x48);
    write(pcMaster, 0xE168, 0xDA);
    write(pcMaster, 0xE169, 0x5A);
    write(pcMaster, 0xE16A, 0xA2);
    write(pcMaster, 0xE16B, 0x36);
    write(pcMaster, 0xE16C, 0x40);
    write(pcMaster, 0xE16D, 0xA0);
    write(pcMaster, 0xE16E, 0x09);
    write(pcMaster, 0xE16F, 0x00);
    write(pcMaster, 0xE170, 0x20);
    write(pcMaster, 0xE171, 0x1D);
    write(pcMaster, 0xE172, 0x40);
    write(pcMaster, 0xE173, 0x80);
    write(pcMaster, 0xE174, 0x33);
    write(pcMaster, 0xE175, 0x8B);
    write(pcMaster, 0xE176, 0x0B);
    write(pcMaster, 0xE177, 0xC2);
    write(pcMaster, 0xE178, 0x30);
    write(pcMaster, 0xE179, 0x48);
    write(pcMaster, 0xE17A, 0xDA);
    write(pcMaster, 0xE17B, 0x5A);
    write(pcMaster, 0xE17C, 0xA2);
    write(pcMaster, 0xE17D, 0x40);
    write(pcMaster, 0xE17E, 0x40);
    write(pcMaster, 0xE17F, 0xA0);
    write(pcMaster, 0xE180, 0x13);
    write(pcMaster, 0xE181, 0x00);
    write(pcMaster, 0xE182, 0x20);
    write(pcMaster, 0xE183, 0x1D);
    write(pcMaster, 0xE184, 0x40);
    write(pcMaster, 0xE185, 0x80);
    write(pcMaster, 0xE186, 0x21);
    write(pcMaster, 0xE187, 0x8B);
    write(pcMaster, 0xE188, 0x0B);
    write(pcMaster, 0xE189, 0xC2);
    write(pcMaster, 0xE18A, 0x30);
    write(pcMaster, 0xE18B, 0x48);
    write(pcMaster, 0xE18C, 0xDA);
    write(pcMaster, 0xE18D, 0x5A);
    write(pcMaster, 0xE18E, 0xA2);
    write(pcMaster, 0xE18F, 0x3C);
    write(pcMaster, 0xE190, 0x40);
    write(pcMaster, 0xE191, 0xA0);
    write(pcMaster, 0xE192, 0x0F);
    write(pcMaster, 0xE193, 0x00);
    write(pcMaster, 0xE194, 0x20);
    write(pcMaster, 0xE195, 0x1D);
    write(pcMaster, 0xE196, 0x40);
    write(pcMaster, 0xE197, 0x80);
    write(pcMaster, 0xE198, 0x0F);
    write(pcMaster, 0xE199, 0x8B);
    write(pcMaster, 0xE19A, 0x0B);
    write(pcMaster, 0xE19B, 0xC2);
    write(pcMaster, 0xE19C, 0x30);
    write(pcMaster, 0xE19D, 0x48);
    write(pcMaster, 0xE19E, 0xDA);
    write(pcMaster, 0xE19F, 0x5A);
    write(pcMaster, 0xE1A0, 0xA9);
    write(pcMaster, 0xE1A1, 0x45);
    write(pcMaster, 0xE1A2, 0x00);
    write(pcMaster, 0xE1A3, 0x20);
    write(pcMaster, 0xE1A4, 0x0E);
    write(pcMaster, 0xE1A5, 0x40);
    write(pcMaster, 0xE1A6, 0x80);
    write(pcMaster, 0xE1A7, 0x00);
    write(pcMaster, 0xE1A8, 0xC2);
    write(pcMaster, 0xE1A9, 0x30);
    write(pcMaster, 0xE1AA, 0x7A);
    write(pcMaster, 0xE1AB, 0xFA);
    write(pcMaster, 0xE1AC, 0x68);
    write(pcMaster, 0xE1AD, 0x2B);
    write(pcMaster, 0xE1AE, 0xAB);
    write(pcMaster, 0xE1AF, 0x40);
    write(pcMaster, 0xE1B0, 0xFF);
    write(pcMaster, 0xE1B1, 0xFF);
    write(pcMaster, 0xFFE4, 0x75);
    write(pcMaster, 0xFFE5, 0xE1);
    write(pcMaster, 0xFFE6, 0x87);
    write(pcMaster, 0xFFE7, 0xE1);
    write(pcMaster, 0xFFE8, 0x63);
    write(pcMaster, 0xFFE9, 0xE1);
    write(pcMaster, 0xFFEA, 0x51);
    write(pcMaster, 0xFFEB, 0xE1);
    write(pcMaster, 0xFFEC, 0x00);
    write(pcMaster, 0xFFED, 0x00);
    write(pcMaster, 0xFFEE, 0x3F);
    write(pcMaster, 0xFFEF, 0xE1);
    write(pcMaster, 0xFFF0, 0x00);
    write(pcMaster, 0xFFF1, 0x00);
    write(pcMaster, 0xFFF2, 0x00);
    write(pcMaster, 0xFFF3, 0x00);
    write(pcMaster, 0xFFF4, 0x99);
    write(pcMaster, 0xFFF5, 0xE1);
    write(pcMaster, 0xFFF6, 0x00);
    write(pcMaster, 0xFFF7, 0x00);
    write(pcMaster, 0xFFF8, 0x99);
    write(pcMaster, 0xFFF9, 0xE1);
    write(pcMaster, 0xFFFA, 0x99);
    write(pcMaster, 0xFFFB, 0xE1);
    write(pcMaster, 0xFFFC, 0x2B);
    write(pcMaster, 0xFFFD, 0xE1);
    write(pcMaster, 0xFFFE, 0x99);
    write(pcMaster, 0xFFFF, 0xE1);
    
    write(pcMaster, 0xFFE2, 0xFF);

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
            strcpy(szLine1, "Half-Cycle      ");
        }

        char szLine2[17];
        if ((iState == 0) || (iState == 1))
        {            
            memset(szLine2, ' ', 16);
            szLine2[16] = '\0';
            sprintf(szLine2, "Cycle: %i", iTick / 2);
            szLine2[strlen(szLine2)] = ' ';
        }
        else
        {
            strcpy(szLine2, "Free running    ");
        }


        put_lines(&sLCDPins, szLine1, szLine2);

        cKeypad.Read();
        char c = cKeypad.GetPressed();

        if (c == '1')
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
            iTick = 0;
           cMaster.Reset(true);
           sleep_us_high_power(10000);
           cMaster.Reset(false);
        }
        else if (c == '0')
        {
            if (!((iState == 0) || (iState == 1)))
            {
                iState = 0;
            }
            write_program(&cMaster);
            iFrequency = calculate_frequency(delay);
            cMaster.BusEnable(true);
        }
        else if (c == '2')
        {
            cMaster.FreeClock(false);
            iState = 0;
            delay = 250'000;
            iFrequency = calculate_frequency(delay);
        }
        else if (c == '3')
        {
            cMaster.FreeClock(false);
            iState = 0;
            delay = 20'000;
            iFrequency = calculate_frequency(delay);
        }
        else if (c == '4')
        {
            cMaster.FreeClock(false);
            iState = 0;
            delay = 2'500;
            iFrequency = calculate_frequency(delay);
        }
        else if (c == '5')
        {
            iFrequency = change_frequency(&sLTCPins, &cMaster, 1500);
            iState = 2;
        }
        else if (c == '6')
        {
            iFrequency = change_frequency(&sLTCPins, &cMaster, 16'750'000);
            iState = 2;
        }
        else if (c == '7')
        {
            iFrequency = change_frequency(&sLTCPins, &cMaster, 35'500'000);
            iState = 2;
        }
        else if (c == '8')
        {
            iFrequency = change_frequency(&sLTCPins, &cMaster, 36'000'000);
            iState = 2;
        }
        else if (c == '9')
        {
            iFrequency = change_frequency(&sLTCPins, &cMaster, 36'500'000);
            iState = 2;
        }
        sleep_us_high_power(500);

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

    SMAX5102Pins    sMaxPins;
    bool            bLed;

    bLed = true;
    sMaxPins.Init(15, 14, 2, 3, 4, 5, 6, 7, 8, 9);
    init_max5102(&sMaxPins);

    int iMaxMask =  make_max5102_mask(&sMaxPins, true, true, 0xff);
    int iZero =  make_max5102_mask(&sMaxPins, false, true, 0);
    float fValue = 0;
    float fDir = 0.5f;
    for (;;)
    {
        int iValue = (int)fValue;
        int iValueB = 0xff - iValue;

        int iWriteANo =  make_max5102_mask(&sMaxPins, false, true, iValue);
        int iWriteAYes =  make_max5102_mask(&sMaxPins, false, false, iValue);
        int iWriteBNo =  make_max5102_mask(&sMaxPins, true, true, iValueB);
        int iWriteBYes =  make_max5102_mask(&sMaxPins, true, false, iValueB);


        gpio_put_masked(iMaxMask, iWriteANo);
        gpio_put_masked(iMaxMask, iWriteAYes);
        gpio_put_masked(iMaxMask, iWriteANo);
        gpio_put_masked(iMaxMask, iWriteBNo);
        gpio_put_masked(iMaxMask, iWriteBYes);
        gpio_put_masked(iMaxMask, iWriteBNo);
        fValue += fDir;
        gpio_put_masked(iMaxMask, iZero);
        if (fValue >= 0x100)
        {
            fValue = 0xff;
            fDir = -fDir;
        }
        else if (fValue < 0)
        {
            fValue = 0;
            fDir = -fDir;
        }

        bLed = !bLed;
        gpio_put(25, bLed);
        sleep_us_high_power(58);
    }

    //do_uart_slave(0, 1, 115200);
    // sleep_us_high_power(250'000);

    // do_uart_master(0, 1, 115200);
    gpio_put(25, false);
}


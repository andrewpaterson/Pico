#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "tusb.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "GeneralPins.h"
#include "PrimitiveTypes.h"
#include "StringHelper.h"


#define ENABLE_GPIO             28
#define ONBOARD_LED             25

#define PWR_12V                 0
#define PWR_5V_A                1
#define PWR_5V_B                2
#define PWR_5V_C                3
#define PWR_GND_A               4
#define PWR_GND_B               5
#define PWR_GND_C               6
#define PWR_GND_D               7

#define DATA_LINE_0             17
#define DATA_LINE_1             18
#define DATA_LINE_2             19
#define DATA_LINE_3             20
#define DATA_LINE_4             21
#define DATA_LINE_5             22
#define DATA_LINE_6             26
#define DATA_LINE_7             27

uint32 aiData_8_15[] = { DATA_LINE_7, DATA_LINE_6, DATA_LINE_5, DATA_LINE_4, DATA_LINE_3, DATA_LINE_2, DATA_LINE_1, DATA_LINE_0 };
uint32 aiData_0_7[] =  { DATA_LINE_7, DATA_LINE_6, DATA_LINE_5, DATA_LINE_4, DATA_LINE_3, DATA_LINE_2, DATA_LINE_1, DATA_LINE_0 };


#define ADDR_LINE_ENABLE        8
#define ADDR_LINE_0             9   //  1
#define ADDR_LINE_1             10  //  2
#define ADDR_LINE_2             11  //  4
#define ADDR_LINE_3             12  //  8
#define ADDR_LINE_4             13  // 10  (high - write, low - read)
#define ADDR_LINE_5             14  // 20

uint32 aiAddress_0_5[] = { ADDR_LINE_0, ADDR_LINE_1, ADDR_LINE_2, ADDR_LINE_3, ADDR_LINE_4, ADDR_LINE_5 };


#define READ_IN                 15
#define WRITE_OUT               16

//Addresses
#define ADDRESS_READ_GPIO_56_63     0x00
#define ADDRESS_READ_GPIO_48_55     0x01
#define ADDRESS_READ_GPIO_40_47     0x02
#define ADDRESS_READ_GPIO_32_39     0x03
#define ADDRESS_READ_GPIO_24_31     0x04
#define ADDRESS_READ_GPIO_16_23     0x05
#define ADDRESS_READ_GPIO__8_15     0x06
#define ADDRESS_READ_GPIO__0__7     0x07

#define ADDRESS_WRITE_GPIO_56_63    0x10
#define ADDRESS_WRITE_GPIO_48_55    0x11
#define ADDRESS_WRITE_GPIO_40_47    0x12
#define ADDRESS_WRITE_GPIO_32_39    0x13
#define ADDRESS_WRITE_GPIO_24_31    0x14
#define ADDRESS_WRITE_GPIO_16_23    0x15
#define ADDRESS_WRITE_GPIO__8_15    0x16
#define ADDRESS_WRITE_GPIO__0__7    0x17

#define ADDRESS_OUTPUT_GPIO_56_63   0x18
#define ADDRESS_OUTPUT_GPIO_48_55   0x19
#define ADDRESS_OUTPUT_GPIO_40_47   0x1A
#define ADDRESS_OUTPUT_GPIO_32_39   0x1B
#define ADDRESS_OUTPUT_GPIO_24_31   0x1C
#define ADDRESS_OUTPUT_GPIO_16_23   0x1D
#define ADDRESS_OUTPUT_GPIO__8_15   0x1E
#define ADDRESS_OUTPUT_GPIO__0__7   0x1F

#define INPUT_BUFFER_SIZE   256


void BlinkLed(uint32 iMicrosecondDelay)
{
    bool bLed = true;

    for (;;)
    {
        gpio_put(ONBOARD_LED, bLed);
        busy_wait_us_32(iMicrosecondDelay);

        bLed = !bLed;
    }
}


bool gbDataRead = true;


void SetDataToInput()
{
    if (!gbDataRead)
    {
        gpio_set_dir_in_masked( 1 << DATA_LINE_0 | 
                                1 << DATA_LINE_1 |
                                1 << DATA_LINE_2 |
                                1 << DATA_LINE_3 |
                                1 << DATA_LINE_4 |
                                1 << DATA_LINE_5 |
                                1 << DATA_LINE_6 |
                                1 << DATA_LINE_7);
        gbDataRead = true;
    }
}


void SetDataToOutput()
{
    if (gbDataRead)
    {
        gpio_set_dir_out_masked(1 << DATA_LINE_0 | 
                                1 << DATA_LINE_1 |
                                1 << DATA_LINE_2 |
                                1 << DATA_LINE_3 |
                                1 << DATA_LINE_4 |
                                1 << DATA_LINE_5 |
                                1 << DATA_LINE_6 |
                                1 << DATA_LINE_7);
        gbDataRead = false;                    
    }
}


uint32 MakeAddress(uint32 uiAddress, bool bEnable)
{
    uint32 uiAddressOnPins = ( uiAddress & 0x01 ? (1ul << aiAddress_0_5[0]) : 0) | 
                                (uiAddress & 0x02 ? (1ul << aiAddress_0_5[1]) : 0) | 
                                (uiAddress & 0x04 ? (1ul << aiAddress_0_5[2]) : 0) | 
                                (uiAddress & 0x08 ? (1ul << aiAddress_0_5[3]) : 0) | 
                                (uiAddress & 0x10 ? (1ul << aiAddress_0_5[4]) : 0) |
                                (bEnable ? (1ul << ADDR_LINE_ENABLE) : 0);
    return uiAddressOnPins;
}


uint32 guiWriteDataMask = (   1 << DATA_LINE_0 | 
                            1 << DATA_LINE_1 |
                            1 << DATA_LINE_2 |
                            1 << DATA_LINE_3 |
                            1 << DATA_LINE_4 |
                            1 << DATA_LINE_5 |
                            1 << DATA_LINE_6 |
                            1 << DATA_LINE_7 |
                            1 << ADDR_LINE_ENABLE |
                            1 << ADDR_LINE_0 |
                            1 << ADDR_LINE_1 |
                            1 << ADDR_LINE_2 |
                            1 << ADDR_LINE_3 |
                            1 << ADDR_LINE_4 |
                            1 << ADDR_LINE_5);

uint32 guiReadDataMask = (1 << ADDR_LINE_ENABLE |
                        1 << ADDR_LINE_0 |
                        1 << ADDR_LINE_1 |
                        1 << ADDR_LINE_2 |
                        1 << ADDR_LINE_3 |
                        1 << ADDR_LINE_4 |
                        1 << ADDR_LINE_5);


void WriteData(uint32 uiAddress, uint32 uiData)
{
    uint32    uiDataOnPins;
    uint32    uiAddressOnPins;

    if (uiAddress & 1 == 1)
    {
        uiDataOnPins = make_8bit_mask(aiData_8_15, uiData);
    }
    else
    {
        uiDataOnPins = make_8bit_mask(aiData_0_7, uiData);
    }

    SetDataToOutput();
    uiAddressOnPins = MakeAddress(uiAddress, true);
    gpio_put_masked(guiWriteDataMask, uiAddressOnPins | uiDataOnPins);

    uiAddressOnPins = MakeAddress(uiAddress, false);
    gpio_put_masked(guiWriteDataMask, uiAddressOnPins | uiDataOnPins);
}


void PulseWriteData(void)
{
    gpio_put(WRITE_OUT, true);
    gpio_put(WRITE_OUT, false);
}


void PulseReadData(void)
{
    gpio_put(READ_IN, true);
    gpio_put(READ_IN, false);
}


uint32 ReadData(uint32 uiAddress)
{
    uint32    uiDataOnPins;
    uint32    uiAddressOnPins;
    uint32    uiValue;

    SetDataToInput();

    uiAddressOnPins = MakeAddress(uiAddress, true);
    gpio_put_masked(guiReadDataMask, uiAddressOnPins);
    for (size i = 0; i < 1; i++)
    {
        asm volatile("nop");
    }

    uiDataOnPins = gpio_get_all();
    uiAddressOnPins = MakeAddress(uiAddress, false);
    gpio_put_masked(guiReadDataMask, uiAddressOnPins);
    
    if (uiAddress & 1 == 1)
    {
        uiValue =   (uiDataOnPins & (1ul << aiData_8_15[0]) ? 1 << 0 : 0) | 
                    (uiDataOnPins & (1ul << aiData_8_15[1]) ? 1 << 1 : 0) | 
                    (uiDataOnPins & (1ul << aiData_8_15[2]) ? 1 << 2 : 0) | 
                    (uiDataOnPins & (1ul << aiData_8_15[3]) ? 1 << 3 : 0) | 
                    (uiDataOnPins & (1ul << aiData_8_15[4]) ? 1 << 4 : 0) | 
                    (uiDataOnPins & (1ul << aiData_8_15[5]) ? 1 << 5 : 0) | 
                    (uiDataOnPins & (1ul << aiData_8_15[6]) ? 1 << 6 : 0) | 
                    (uiDataOnPins & (1ul << aiData_8_15[7]) ? 1 << 7 : 0);

    }
    else
    {
        uiValue =   (uiDataOnPins & (1ul << aiData_0_7[0]) ? 1 << 0 : 0) | 
                    (uiDataOnPins & (1ul << aiData_0_7[1]) ? 1 << 1 : 0) | 
                    (uiDataOnPins & (1ul << aiData_0_7[2]) ? 1 << 2 : 0) | 
                    (uiDataOnPins & (1ul << aiData_0_7[3]) ? 1 << 3 : 0) | 
                    (uiDataOnPins & (1ul << aiData_0_7[4]) ? 1 << 4 : 0) | 
                    (uiDataOnPins & (1ul << aiData_0_7[5]) ? 1 << 5 : 0) | 
                    (uiDataOnPins & (1ul << aiData_0_7[6]) ? 1 << 6 : 0) | 
                    (uiDataOnPins & (1ul << aiData_0_7[7]) ? 1 << 7 : 0);
    }

    return uiValue;
}


void SetAllToOutput(unsigned char uiValue)
{
    for (size j = ADDRESS_OUTPUT_GPIO_56_63; j <= ADDRESS_OUTPUT_GPIO__0__7; j++)
    {
        WriteData(j, uiValue);
    }
}


void SetAllToData(unsigned char uiValue)
{
    for (size j = ADDRESS_WRITE_GPIO_56_63; j <= ADDRESS_WRITE_GPIO__0__7; j++)
    {
        WriteData(j, uiValue);
    }
}


void TestRead(bool bReadLeft)
{
    uint32    uiNoOutput0 = bReadLeft ? ADDRESS_OUTPUT_GPIO__0__7 : ADDRESS_OUTPUT_GPIO_32_39;
    uint32    uiNoOutput1 = bReadLeft ? ADDRESS_OUTPUT_GPIO__8_15 : ADDRESS_OUTPUT_GPIO_40_47;
    uint32    uiNoOutput2 = bReadLeft ? ADDRESS_OUTPUT_GPIO_16_23 : ADDRESS_OUTPUT_GPIO_48_55;
    uint32    uiNoOutput3 = bReadLeft ? ADDRESS_OUTPUT_GPIO_24_31 : ADDRESS_OUTPUT_GPIO_56_63;
    uint32    uiOutput0 = bReadLeft ? ADDRESS_OUTPUT_GPIO_32_39 : ADDRESS_OUTPUT_GPIO__0__7;
    uint32    uiOutput1 = bReadLeft ? ADDRESS_OUTPUT_GPIO_40_47 : ADDRESS_OUTPUT_GPIO__8_15;
    uint32    uiOutput2 = bReadLeft ? ADDRESS_OUTPUT_GPIO_48_55 : ADDRESS_OUTPUT_GPIO_16_23;
    uint32    uiOutput3 = bReadLeft ? ADDRESS_OUTPUT_GPIO_56_63 : ADDRESS_OUTPUT_GPIO_24_31;

    uint32    uiRead0 = bReadLeft ? ADDRESS_READ_GPIO__0__7 : ADDRESS_READ_GPIO_32_39;
    uint32    uiRead1 = bReadLeft ? ADDRESS_READ_GPIO__8_15 : ADDRESS_READ_GPIO_40_47;
    uint32    uiRead2 = bReadLeft ? ADDRESS_READ_GPIO_16_23 : ADDRESS_READ_GPIO_48_55;
    uint32    uiRead3 = bReadLeft ? ADDRESS_READ_GPIO_24_31 : ADDRESS_READ_GPIO_56_63;

    uint32    uiWrite0 = bReadLeft ? ADDRESS_WRITE_GPIO_32_39 : ADDRESS_WRITE_GPIO__0__7;
    uint32    uiWrite1 = bReadLeft ? ADDRESS_WRITE_GPIO_40_47 : ADDRESS_WRITE_GPIO__8_15;
    uint32    uiWrite2 = bReadLeft ? ADDRESS_WRITE_GPIO_48_55 : ADDRESS_WRITE_GPIO_16_23;
    uint32    uiWrite3 = bReadLeft ? ADDRESS_WRITE_GPIO_56_63 : ADDRESS_WRITE_GPIO_24_31;

    SetAllToData(0x00);

    WriteData(uiNoOutput0, 0x00);
    WriteData(uiNoOutput1, 0x00);
    WriteData(uiNoOutput2, 0x00);
    WriteData(uiNoOutput3, 0x00);
    WriteData(uiOutput0, 0xff);
    WriteData(uiOutput1, 0xff);
    WriteData(uiOutput2, 0Xff);
    WriteData(uiOutput3, 0xff);
    PulseWriteData();

    uint32    uiValue;
    for (;;)
    {
        PulseReadData();
        uiValue = ReadData(uiRead0);
        WriteData(uiWrite0, uiValue);

        uiValue = ReadData(uiRead1);
        WriteData(uiWrite1, uiValue);

        uiValue = ReadData(uiRead2);
        WriteData(uiWrite2, uiValue);

        uiValue = ReadData(uiRead3);
        WriteData(uiWrite3, uiValue);
        PulseWriteData();
    }
}


void TestWrite(void)
{
    for (;;)
    {
        SetAllToOutput(0xff);
        PulseWriteData();
        for (size i = 0; i < 64; i++)
        {
            uint32 iDataOffset;
            uint32 iAddress;
            SetAllToData(0x00);

            iAddress = i / 8;
            iDataOffset = 1 << (7 - (i % 8));
            WriteData(ADDRESS_WRITE_GPIO__0__7 - iAddress, iDataOffset);
            PulseWriteData();
            sleep_ms(40);
        }

        SetAllToData(0xff);
        SetAllToOutput(0x00);
        PulseWriteData();
        for (size i = 63; i >= 0; i--)
        {
            uint32 iDataOffset;
            uint32 iAddress;
            SetAllToOutput(0x00);

            iAddress = i / 8;
            iDataOffset = 1 << (7 - (i % 8));
            WriteData(ADDRESS_OUTPUT_GPIO__0__7 - iAddress, iDataOffset);
            PulseWriteData();
            sleep_ms(40);
        }
    }
}


void InitPicoPins(void)
{
    stdio_init_all();
    gpio_put_all(0);

    gpio_init(ONBOARD_LED);
    gpio_set_dir(ONBOARD_LED, GPIO_OUT);

    gpio_init(PWR_12V);
    gpio_init(PWR_5V_A);
    gpio_init(PWR_5V_B);
    gpio_init(PWR_5V_C);
    gpio_init(PWR_GND_A);
    gpio_init(PWR_GND_B);
    gpio_init(PWR_GND_C);
    gpio_init(PWR_GND_D);
    gpio_set_dir(PWR_12V, GPIO_OUT);
    gpio_set_dir(PWR_5V_A, GPIO_OUT);    
    gpio_set_dir(PWR_5V_B, GPIO_OUT);
    gpio_set_dir(PWR_5V_C, GPIO_OUT);    
    gpio_set_dir(PWR_GND_A, GPIO_OUT);
    gpio_set_dir(PWR_GND_B, GPIO_OUT);    
    gpio_set_dir(PWR_GND_C, GPIO_OUT);
    gpio_set_dir(PWR_GND_D, GPIO_OUT);   

    gpio_init(ENABLE_GPIO);
    gpio_init(ADDR_LINE_ENABLE);
    gpio_init(ADDR_LINE_0);
    gpio_init(ADDR_LINE_1);
    gpio_init(ADDR_LINE_2);
    gpio_init(ADDR_LINE_3);
    gpio_init(ADDR_LINE_4);
    gpio_init(ADDR_LINE_5);
    gpio_init(READ_IN);
    gpio_init(WRITE_OUT);
    gpio_set_dir(ENABLE_GPIO, GPIO_OUT);
    gpio_set_dir(ADDR_LINE_ENABLE, GPIO_OUT);
    gpio_set_dir(ADDR_LINE_0, GPIO_OUT);
    gpio_set_dir(ADDR_LINE_1, GPIO_OUT);
    gpio_set_dir(ADDR_LINE_2, GPIO_OUT);
    gpio_set_dir(ADDR_LINE_3, GPIO_OUT);
    gpio_set_dir(ADDR_LINE_4, GPIO_OUT);
    gpio_set_dir(ADDR_LINE_5, GPIO_OUT);
    gpio_set_dir(READ_IN, GPIO_OUT);
    gpio_set_dir(WRITE_OUT, GPIO_OUT);

    gpio_init(DATA_LINE_0);
    gpio_init(DATA_LINE_1);
    gpio_init(DATA_LINE_2);
    gpio_init(DATA_LINE_3);
    gpio_init(DATA_LINE_4);
    gpio_init(DATA_LINE_5);
    gpio_init(DATA_LINE_6);
    gpio_init(DATA_LINE_7);
    gpio_set_dir(DATA_LINE_0, GPIO_IN);
    gpio_set_dir(DATA_LINE_1, GPIO_IN);
    gpio_set_dir(DATA_LINE_2, GPIO_IN);
    gpio_set_dir(DATA_LINE_3, GPIO_IN);
    gpio_set_dir(DATA_LINE_4, GPIO_IN);
    gpio_set_dir(DATA_LINE_5, GPIO_IN);
    gpio_set_dir(DATA_LINE_6, GPIO_IN);
    gpio_set_dir(DATA_LINE_7, GPIO_IN);
}


void ExecuteCommand(char* pszCommand, size uiLength)
{
    //* X - Use ASCII hex
    //* XX - Use 8bit bytes
    //Wx..x - Write data bits e.g. Dff00ff00ff00ff00ff00ff00ff00ff00 writes 1s to bits 127 to 120 then 0s to bits 119 to 112 etc... 
    //                            Dff writes 1s to bits 7 to 0.
    //* W_ox..x - Write data bits with byte offset e.g. D_1ff00 writes 1s to bits 23 to 16 then 0s to bits 15 to 8.
    //                                               D_0f writes 1s to bits 3 to 0 (same as Df)
    //W - Write all data bits with 0.
    //Ox..x - Set GPIO as outputs from bits e.g. Off00ff00ff00ff00ff00ff00ff00ff00 sets bits 127 to 120 to outputs then sets bits 119 to 112 to inputs etc... 
    //O_ox..x - same as Wo_xxx but sets outputs.
    //O - Set all GPIO as inputs
    //Ro_p - Latch ALL input bits.  Return values from byte o to byte p inclusive.  GPIO set as output are also returned and should be masked off.
    //       e.g. R0_7 return all 128 bits of GPIO
    //Ro - Same as Ro_p but for a single byte.
    //R - Latch ALL input bits.  Return all values from byte 0 to 15 inclusive.
    
    //Note: you should call W before calling O to ensure the correct values are present when the GPIO is set to output.
    //Note: the first byte returned is the high byte.  The first nybble returned is the high nybble.
    //Note: Offsets o and p are always ASCII hex even if mode is XX
    
    if (uiLength == 0)
    {

    }
    else if (uiLength == 1)
    {
        if (pszCommand[0] == 'X')
        {

        }
        else if (pszCommand[0] == 'W')
        {

        }
        else if (pszCommand[0] == 'O')
        {

        }
        else if (pszCommand[0] == 'R')
        {

        }
    }
    else
    {
        if (MemCmp("XX", 2, pszCommand, uiLength) == 0)
        {

        }
        else if (MemCmp("W_", 2, pszCommand, 2) == 0)
        {

        }
        else if (MemCmp("O_", 2, pszCommand, 2) == 0)
        {

        }
        else if (pszCommand[0] == 'W')
        {

        }
        else if (pszCommand[0] == 'O')
        {

        }
        else if (pszCommand[0] == 'R')
        {

        }
    }
}


int main(void)
{
    InitPicoPins();

    gpio_put(ENABLE_GPIO, true);
    sleep_us(0);
    gpio_put(ENABLE_GPIO, false);

    gpio_put(ONBOARD_LED, true);

    char szInput[INPUT_BUFFER_SIZE];
    size  uiInputIndex;

    memset(szInput, 0, INPUT_BUFFER_SIZE);
    uiInputIndex = 0;

    stdio_usb_init();
    while (!tusb_inited()) {
        tight_loop_contents(); // Wait for USB to initialize
    }

    // Main loop
    size x = 0;
    while (1) {
        // Read incoming character from USB serial
        int32 c = getchar_timeout_us(0); // Timeout after 10ms
        if (c != PICO_ERROR_TIMEOUT) 
        {
            if (c == '\n') 
            {
                szInput[uiInputIndex] = '\0';
                ExecuteCommand(szInput, uiInputIndex);
                uiInputIndex = 0;
            }
            else
            {
                szInput[uiInputIndex] = c;
                uiInputIndex++;
            }
        }
        if (x == 1000000)
        {
            printf("Pico USB UART\n");
            x = 0;
        }
        x++;
    }
    
    BlinkLed(100000);
    return 0;
}


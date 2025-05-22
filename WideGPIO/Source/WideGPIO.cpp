#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "GeneralPins.h"


#define ENABLE_GPIO             28

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

int aiData_8_15[] = { DATA_LINE_0, DATA_LINE_1, DATA_LINE_2, DATA_LINE_3, DATA_LINE_4, DATA_LINE_5, DATA_LINE_6, DATA_LINE_7 };
int aiData_0_7[] =  { DATA_LINE_7, DATA_LINE_6, DATA_LINE_5, DATA_LINE_4, DATA_LINE_3, DATA_LINE_2, DATA_LINE_1, DATA_LINE_0 };


#define ADDR_LINE_ENABLE        8
#define ADDR_LINE_0             9
#define ADDR_LINE_1             10
#define ADDR_LINE_2             11
#define ADDR_LINE_3             12
#define ADDR_LINE_4             13
#define ADDR_LINE_5             14

int aiAddress_0_4[] = { ADDR_LINE_0, ADDR_LINE_1, ADDR_LINE_2, ADDR_LINE_3, ADDR_LINE_4 };


#define READ_IN                 15
#define WRITE_OUT               16

//Addresses
//A4:0 A3:0 A2..A0:0..7
#define ADDRESS_READ_GPIO_56_63     0x00
#define ADDRESS_READ_GPIO_48_55     0x01
#define ADDRESS_READ_GPIO_40_47     0x02
#define ADDRESS_READ_GPIO_32_39     0x03
#define ADDRESS_READ_GPIO_24_31     0x04
#define ADDRESS_READ_GPIO_16_23     0x05
#define ADDRESS_READ_GPIO__8_15     0x06
#define ADDRESS_READ_GPIO__0__7     0x07
//A4:0 A3:1 A2..A0:0..7
#define ADDRESS_WRITE_GPIO_56_63    0x10
#define ADDRESS_WRITE_GPIO_48_55    0x11
#define ADDRESS_WRITE_GPIO_40_47    0x12
#define ADDRESS_WRITE_GPIO_32_39    0x13
#define ADDRESS_WRITE_GPIO_24_31    0x14
#define ADDRESS_WRITE_GPIO_16_23    0x15
#define ADDRESS_WRITE_GPIO__8_15    0x16
#define ADDRESS_WRITE_GPIO__0__7    0x17
//A4:1 A3:0 A2..A0:0..7
#define ADDRESS_OUTPUT_GPIO_56_63   0x18
#define ADDRESS_OUTPUT_GPIO_48_55   0x19
#define ADDRESS_OUTPUT_GPIO_40_47   0x1A
#define ADDRESS_OUTPUT_GPIO_32_39   0x1B
#define ADDRESS_OUTPUT_GPIO_24_31   0x1C
#define ADDRESS_OUTPUT_GPIO_16_23   0x1D
#define ADDRESS_OUTPUT_GPIO__8_15   0x1E
#define ADDRESS_OUTPUT_GPIO__0__7   0x1F


void blink_led(int iMicrosecondDelay)
{
    bool bLed = true;

    for (;;)
    {
        gpio_put(25, bLed);
        busy_wait_us_32(iMicrosecondDelay);

        bLed = !bLed;
    }
}


bool gbDataRead = true;


void set_data_input()
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


void set_data_output()
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


uint32_t make_address(uint32_t uiAddress, bool bEnable)
{
    uint32_t uiAddressOnPins = ( uiAddress & 0x01 ? (1ul << aiAddress_0_4[0]) : 0) | 
                                (uiAddress & 0x02 ? (1ul << aiAddress_0_4[1]) : 0) | 
                                (uiAddress & 0x04 ? (1ul << aiAddress_0_4[2]) : 0) | 
                                (uiAddress & 0x08 ? (1ul << aiAddress_0_4[3]) : 0) | 
                                (uiAddress & 0x10 ? (1ul << aiAddress_0_4[4]) : 0) |
                                (bEnable ? (1ul << ADDR_LINE_ENABLE) : 0);
    return uiAddressOnPins;
}


uint guiWriteDataMask = (   1 << DATA_LINE_0 | 
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
                            1 << ADDR_LINE_4 );

uint guiReadDataMask = (1 << ADDR_LINE_ENABLE |
                        1 << ADDR_LINE_0 |
                        1 << ADDR_LINE_1 |
                        1 << ADDR_LINE_2 |
                        1 << ADDR_LINE_3 |
                        1 << ADDR_LINE_4 );


void write_data(uint32_t uiAddress, uint32_t uiData)
{
    uint32_t    uiDataOnPins;
    uint32_t    uiAddressOnPins;

    if (uiAddress & 1 == 1)
    {
        uiDataOnPins = make_8bit_mask(aiData_8_15, uiData);
    }
    else
    {
        uiDataOnPins = make_8bit_mask(aiData_0_7, uiData);
    }

    set_data_output();
    uiAddressOnPins = make_address(uiAddress, true);
    gpio_put_masked(guiWriteDataMask, uiAddressOnPins | uiDataOnPins);

    uiAddressOnPins = make_address(uiAddress, false);
    gpio_put_masked(guiWriteDataMask, uiAddressOnPins | uiDataOnPins);
}


void write_data(void)
{
    gpio_put(WRITE_OUT, true);
    gpio_put(WRITE_OUT, false);
}


void read_data(void)
{
    gpio_put(READ_IN, true);
    gpio_put(READ_IN, false);
}


uint32_t read_data(uint32_t uiAddress)
{
    uint32_t    uiDataOnPins;
    uint32_t    uiAddressOnPins;
    uint32_t    uiValue;

    set_data_input();

    uiAddressOnPins = make_address(uiAddress, true);
    gpio_put_masked(guiReadDataMask, uiAddressOnPins);
    for (int i = 0; i < 1; i++)
    {
        asm volatile("nop");
    }

    uiDataOnPins = gpio_get_all();
    uiAddressOnPins = make_address(uiAddress, false);
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


void set_all_output(unsigned char uiValue)
{
    for (int j = ADDRESS_OUTPUT_GPIO_56_63; j <= ADDRESS_OUTPUT_GPIO__0__7; j++)
    {
        write_data(j, uiValue);
    }
}


void set_all_data(unsigned char uiValue)
{
    for (int j = ADDRESS_WRITE_GPIO_56_63; j <= ADDRESS_WRITE_GPIO__0__7; j++)
    {
        write_data(j, uiValue);
    }
}


void blink_all(int times, int wait)
{
    set_all_output(0xff);

    for (int i = 0; i < times; i++)
    {
        set_all_data(0x00);
        write_data();
        sleep_ms(wait);

        set_all_data(0xff);
        write_data();
        sleep_ms(wait);
    }
}


void test_read(bool bReadLeft)
{
    uint32_t    uiNoOutput0 = bReadLeft ? ADDRESS_OUTPUT_GPIO__0__7 : ADDRESS_OUTPUT_GPIO_32_39;
    uint32_t    uiNoOutput1 = bReadLeft ? ADDRESS_OUTPUT_GPIO__8_15 : ADDRESS_OUTPUT_GPIO_40_47;
    uint32_t    uiNoOutput2 = bReadLeft ? ADDRESS_OUTPUT_GPIO_16_23 : ADDRESS_OUTPUT_GPIO_48_55;
    uint32_t    uiNoOutput3 = bReadLeft ? ADDRESS_OUTPUT_GPIO_24_31 : ADDRESS_OUTPUT_GPIO_56_63;
    uint32_t    uiOutput0 = bReadLeft ? ADDRESS_OUTPUT_GPIO_32_39 : ADDRESS_OUTPUT_GPIO__0__7;
    uint32_t    uiOutput1 = bReadLeft ? ADDRESS_OUTPUT_GPIO_40_47 : ADDRESS_OUTPUT_GPIO__8_15;
    uint32_t    uiOutput2 = bReadLeft ? ADDRESS_OUTPUT_GPIO_48_55 : ADDRESS_OUTPUT_GPIO_16_23;
    uint32_t    uiOutput3 = bReadLeft ? ADDRESS_OUTPUT_GPIO_56_63 : ADDRESS_OUTPUT_GPIO_24_31;

    uint32_t    uiRead0 = bReadLeft ? ADDRESS_READ_GPIO__0__7 : ADDRESS_READ_GPIO_32_39;
    uint32_t    uiRead1 = bReadLeft ? ADDRESS_READ_GPIO__8_15 : ADDRESS_READ_GPIO_40_47;
    uint32_t    uiRead2 = bReadLeft ? ADDRESS_READ_GPIO_16_23 : ADDRESS_READ_GPIO_48_55;
    uint32_t    uiRead3 = bReadLeft ? ADDRESS_READ_GPIO_24_31 : ADDRESS_READ_GPIO_56_63;

    uint32_t    uiWrite0 = bReadLeft ? ADDRESS_WRITE_GPIO_32_39 : ADDRESS_WRITE_GPIO__0__7;
    uint32_t    uiWrite1 = bReadLeft ? ADDRESS_WRITE_GPIO_40_47 : ADDRESS_WRITE_GPIO__8_15;
    uint32_t    uiWrite2 = bReadLeft ? ADDRESS_WRITE_GPIO_48_55 : ADDRESS_WRITE_GPIO_16_23;
    uint32_t    uiWrite3 = bReadLeft ? ADDRESS_WRITE_GPIO_56_63 : ADDRESS_WRITE_GPIO_24_31;

    set_all_data(0x00);

    write_data(uiNoOutput0, 0x00);
    write_data(uiNoOutput1, 0x00);
    write_data(uiNoOutput2, 0x00);
    write_data(uiNoOutput3, 0x00);
    write_data(uiOutput0, 0xff);
    write_data(uiOutput1, 0xff);
    write_data(uiOutput2, 0Xff);
    write_data(uiOutput3, 0xff);
    write_data();

    uint32_t    uiValue;
    for (;;)
    {
        read_data();
        uiValue = read_data(uiRead0);
        write_data(uiWrite0, uiValue);

        uiValue = read_data(uiRead1);
        write_data(uiWrite1, uiValue);

        uiValue = read_data(uiRead2);
        write_data(uiWrite2, uiValue);

        uiValue = read_data(uiRead3);
        write_data(uiWrite3, uiValue);
        write_data();
    }
}


void test_write(void)
{
    for (;;)
    {
        blink_all(2, 150);

        set_all_output(0xff);
        write_data();
        for (int i = 0; i < 64; i++)
        {
            int iDataOffset;
            int iAddress;
            set_all_data(0x00);

            iAddress = i / 8;
            iDataOffset = 1 << (7 - (i % 8));
            write_data(ADDRESS_WRITE_GPIO__0__7 - iAddress, iDataOffset);
            write_data();
            sleep_ms(40);
        }

        blink_all(3, 150);

        set_all_data(0xff);
        set_all_output(0x00);
        write_data();
        for (int i = 63; i >= 0; i--)
        {
            int iDataOffset;
            int iAddress;
            set_all_output(0x00);

            iAddress = i / 8;
            iDataOffset = 1 << (7 - (i % 8));
            write_data(ADDRESS_OUTPUT_GPIO__0__7 - iAddress, iDataOffset);
            write_data();
            sleep_ms(40);
        }
    }
}


int main() 
{
    stdio_init_all();
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    gpio_put(25, true);

    gpio_init(PWR_12V);
    gpio_init(PWR_5V_A);
    gpio_init(PWR_5V_B);
    gpio_init(PWR_5V_C);
    gpio_init(PWR_GND_A);
    gpio_init(PWR_GND_B);
    gpio_init(PWR_GND_C);
    gpio_init(PWR_GND_D);

    gpio_init(ENABLE_GPIO);
    gpio_init(DATA_LINE_0);
    gpio_init(DATA_LINE_1);
    gpio_init(DATA_LINE_2);
    gpio_init(DATA_LINE_3);
    gpio_init(DATA_LINE_4);
    gpio_init(DATA_LINE_5);
    gpio_init(DATA_LINE_6);
    gpio_init(DATA_LINE_7);
    gpio_init(ADDR_LINE_ENABLE);
    gpio_init(ADDR_LINE_0);
    gpio_init(ADDR_LINE_1);
    gpio_init(ADDR_LINE_2);
    gpio_init(ADDR_LINE_3);
    gpio_init(ADDR_LINE_4);
    gpio_init(ADDR_LINE_5);
    gpio_init(READ_IN);
    gpio_init(WRITE_OUT);

    gpio_put_all(0);
    gpio_set_dir(PWR_12V, GPIO_OUT);
    gpio_set_dir(PWR_5V_A, GPIO_OUT);    
    gpio_set_dir(PWR_5V_B, GPIO_OUT);
    gpio_set_dir(PWR_5V_C, GPIO_OUT);    
    gpio_set_dir(PWR_GND_A, GPIO_OUT);
    gpio_set_dir(PWR_GND_B, GPIO_OUT);    
    gpio_set_dir(PWR_GND_C, GPIO_OUT);
    gpio_set_dir(PWR_GND_D, GPIO_OUT);    

    gpio_set_dir(ENABLE_GPIO, GPIO_OUT);
    gpio_set_dir(ADDR_LINE_ENABLE, GPIO_OUT);
    gpio_set_dir(ADDR_LINE_0, GPIO_OUT);
    gpio_set_dir(ADDR_LINE_1, GPIO_OUT);
    gpio_set_dir(ADDR_LINE_2, GPIO_OUT);
    gpio_set_dir(ADDR_LINE_3, GPIO_OUT);
    gpio_set_dir(ADDR_LINE_4, GPIO_OUT);
    gpio_set_dir(READ_IN, GPIO_OUT);
    gpio_set_dir(WRITE_OUT, GPIO_OUT);

    gpio_put(ENABLE_GPIO, true);
    sleep_us(0);
    gpio_put(ENABLE_GPIO, false);

    gpio_put(25, false);

    //test_read(true);
    test_write();

    blink_led(100000);
    return 0;
}


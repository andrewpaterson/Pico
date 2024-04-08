#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "GeneralPins.h"


#define ENABLE_GPIO             28


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

int aiAddress_0_4[] = { ADDR_LINE_0, ADDR_LINE_1, ADDR_LINE_2, ADDR_LINE_3, ADDR_LINE_4 };


#define READ_IN                 14
#define WRITE_OUT               15

//A4:0 A3:0 A2..A0:0..7
#define ADDRESS_READ_GPIO_56_63     0x00
#define ADDRESS_READ_GPIO_48_55     0x01
#define ADDRESS_READ_GPIO_40_47     0x02
#define ADDRESS_READ_GPIO_32_39     0x03
#define ADDRESS_READ_GPIO_24_31     0x04
#define ADDRESS_READ_GPIO_16_23     0x05
#define ADDRESS_READ_GPIO__8_11     0x06
#define ADDRESS_READ_GPIO__0__7     0x07
//A4:0 A3:1 A2..A0:0..7
#define ADDRESS_WRITE_GPIO_56_63    0x08
#define ADDRESS_WRITE_GPIO_48_55    0x09
#define ADDRESS_WRITE_GPIO_40_47    0x0A
#define ADDRESS_WRITE_GPIO_32_39    0x0B
#define ADDRESS_WRITE_GPIO_24_31    0x0C
#define ADDRESS_WRITE_GPIO_16_23    0x0D
#define ADDRESS_WRITE_GPIO__8_11    0x0E
#define ADDRESS_WRITE_GPIO__0__7    0x0F
//A4:1 A3:0 A2..A0:0..7
#define ADDRESS_OUTPUT_GPIO_56_63   0x10
#define ADDRESS_OUTPUT_GPIO_48_55   0x11
#define ADDRESS_OUTPUT_GPIO_40_47   0x12
#define ADDRESS_OUTPUT_GPIO_32_39   0x13
#define ADDRESS_OUTPUT_GPIO_24_31   0x14
#define ADDRESS_OUTPUT_GPIO_16_23   0x15
#define ADDRESS_OUTPUT_GPIO__8_11   0x16
#define ADDRESS_OUTPUT_GPIO__0__7   0x17


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


uint32_t make_address(uint32_t uiAddress)
{
    uint32_t uiAddressOnPins = ( uiAddress & 0x01 ? (1ul << aiAddress_0_4[0]) : 0) | 
                                (uiAddress & 0x02 ? (1ul << aiAddress_0_4[1]) : 0) | 
                                (uiAddress & 0x04 ? (1ul << aiAddress_0_4[2]) : 0) | 
                                (uiAddress & 0x08 ? (1ul << aiAddress_0_4[3]) : 0) | 
                                (uiAddress & 0x10 ? (1ul << aiAddress_0_4[4]) : 0) |
                                (1ul << ADDR_LINE_ENABLE);
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

    uiAddressOnPins = make_address(uiAddress);

    gpio_put_masked(guiWriteDataMask, uiAddressOnPins | uiDataOnPins);
}


int main() 
{
    stdio_init_all();
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    gpio_put(25, true);

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
    gpio_init(READ_IN);
    gpio_init(WRITE_OUT);

    gpio_put_all(0);
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

    set_data_output();
    gpio_put(25, false);


    int i = 0;
    for (;;)
    {
        write_data(i, 0xff);
        sleep_ms(125);
        i++;
        if (i > 0x17)
        {
            i = 0;
        }
    }

    blink_led(100000);

    return 0;
}


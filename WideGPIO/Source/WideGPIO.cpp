#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"


#define READ_DATA_0__7      8
#define READ_DATA_8__15     9
#define READ_IN             10
#define WRITE_OUT           11
#define WRITE_DATA_0__7     12
#define WRITE_DATA_8__15    13
#define OUT_ENABLE_0__7     14
#define OUT_ENABLE_8__15    15
#define DATA_0              16
#define DATA_1              17
#define DATA_2              18
#define DATA_3              19
#define DATA_4              20
#define DATA_5              21
#define DATA_6              22
#define DATA_7              26

int aiData[] = { DATA_0, DATA_1, DATA_2, DATA_3, DATA_4, DATA_5, DATA_6, DATA_7 };


int doOutput() 
{
    stdio_init_all();
    gpio_init(25);
    gpio_init(READ_DATA_0__7);
    gpio_init(READ_DATA_8__15);
    gpio_init(READ_IN);
    gpio_init(WRITE_OUT);
    gpio_init(WRITE_DATA_0__7);
    gpio_init(WRITE_DATA_8__15);
    gpio_init(OUT_ENABLE_0__7);
    gpio_init(OUT_ENABLE_8__15);
    gpio_init(DATA_0);
    gpio_init(DATA_1);
    gpio_init(DATA_2);
    gpio_init(DATA_3);
    gpio_init(DATA_4);
    gpio_init(DATA_5);
    gpio_init(DATA_6);
    gpio_init(DATA_7);
    gpio_put_all(0);

    gpio_set_dir(25, GPIO_OUT);
    gpio_set_dir(READ_DATA_0__7, GPIO_OUT);
    gpio_set_dir(READ_DATA_8__15, GPIO_OUT);
    gpio_set_dir(READ_IN, GPIO_OUT);
    gpio_set_dir(WRITE_OUT, GPIO_OUT);
    gpio_set_dir(WRITE_DATA_0__7, GPIO_OUT);
    gpio_set_dir(WRITE_DATA_8__15, GPIO_OUT);
    gpio_set_dir(OUT_ENABLE_0__7, GPIO_OUT);
    gpio_set_dir(OUT_ENABLE_8__15, GPIO_OUT);
    gpio_set_dir(DATA_0, GPIO_OUT);
    gpio_set_dir(DATA_1, GPIO_OUT);
    gpio_set_dir(DATA_2, GPIO_OUT);
    gpio_set_dir(DATA_3, GPIO_OUT);
    gpio_set_dir(DATA_4, GPIO_OUT);
    gpio_set_dir(DATA_5, GPIO_OUT);
    gpio_set_dir(DATA_6, GPIO_OUT);
    gpio_set_dir(DATA_7, GPIO_OUT);

    gpio_put(25, true);
    gpio_put(DATA_0, true);
    gpio_put(DATA_1, true);
    gpio_put(DATA_2, true);
    gpio_put(DATA_3, true);
    gpio_put(DATA_4, true);
    gpio_put(DATA_5, true);
    gpio_put(DATA_6, true);
    gpio_put(DATA_7, true);
    gpio_put(OUT_ENABLE_0__7, true);
    gpio_put(OUT_ENABLE_8__15, true);
    gpio_put(WRITE_DATA_0__7, true);
    gpio_put(WRITE_DATA_8__15, true);
    gpio_put(WRITE_OUT, true);
    sleep_ms(1);
    gpio_put(OUT_ENABLE_0__7, false);
    gpio_put(OUT_ENABLE_8__15, false);
    gpio_put(WRITE_DATA_0__7, false);
    gpio_put(WRITE_DATA_8__15, false);
    gpio_put(WRITE_OUT, false);

    int i = 0;
    for (;;)
    {
        sleep_ms(32);
        gpio_put(25, false);

        gpio_put(DATA_0, false);
        gpio_put(DATA_1, false);
        gpio_put(DATA_2, false);
        gpio_put(DATA_3, false);
        gpio_put(DATA_4, false);
        gpio_put(DATA_5, false);
        gpio_put(DATA_6, false);
        gpio_put(DATA_7, false);

        if ((i >= 0) && (i <= 7))
        {
            gpio_put(WRITE_DATA_8__15, true);
            gpio_put(WRITE_DATA_8__15, false);
            gpio_put(aiData[7 - i], true);
            gpio_put(WRITE_DATA_0__7, true);
            gpio_put(WRITE_DATA_0__7, false);
        }
        else if ((i >= 8) && (i <= 15))
        {
            gpio_put(WRITE_DATA_0__7, true);
            gpio_put(WRITE_DATA_0__7, false);
            gpio_put(aiData[i - 8], true);
            gpio_put(WRITE_DATA_8__15, true);
            gpio_put(WRITE_DATA_8__15, false);
        }
        else if ((i >= 16) && (i <= 23))
        {
            gpio_put(WRITE_DATA_0__7, true);
            gpio_put(WRITE_DATA_0__7, false);
            gpio_put(aiData[23 - i], true);
            gpio_put(WRITE_DATA_8__15, true);
            gpio_put(WRITE_DATA_8__15, false);
        }
        else if ((i >= 24) && (i <= 31))
        {
            gpio_put(WRITE_DATA_8__15, true);
            gpio_put(WRITE_DATA_8__15, false);
            gpio_put(aiData[i - 24], true);
            gpio_put(WRITE_DATA_0__7, true);
            gpio_put(WRITE_DATA_0__7, false);
        }

        gpio_put(WRITE_OUT, true);
        gpio_put(WRITE_OUT, false);

        sleep_ms(32);
        gpio_put(25, true);

        i++;
        if (i == 32)
        {
            i = 0;
        }
    }
    return 0;
}


void doGPIO(int iSide)
{
    stdio_init_all();
    gpio_init(25);
    gpio_init(READ_DATA_0__7);
    gpio_init(READ_DATA_8__15);
    gpio_init(READ_IN);
    gpio_init(WRITE_OUT);
    gpio_init(WRITE_DATA_0__7);
    gpio_init(WRITE_DATA_8__15);
    gpio_init(OUT_ENABLE_0__7);
    gpio_init(OUT_ENABLE_8__15);
    gpio_init(DATA_0);
    gpio_init(DATA_1);
    gpio_init(DATA_2);
    gpio_init(DATA_3);
    gpio_init(DATA_4);
    gpio_init(DATA_5);
    gpio_init(DATA_6);
    gpio_init(DATA_7);
    gpio_put_all(0);

    gpio_set_dir(25, GPIO_OUT);
    gpio_set_dir(READ_DATA_0__7, GPIO_OUT);
    gpio_set_dir(READ_DATA_8__15, GPIO_OUT);
    gpio_set_dir(READ_IN, GPIO_OUT);
    gpio_set_dir(WRITE_OUT, GPIO_OUT);
    gpio_set_dir(WRITE_DATA_0__7, GPIO_OUT);
    gpio_set_dir(WRITE_DATA_8__15, GPIO_OUT);
    gpio_set_dir(OUT_ENABLE_0__7, GPIO_OUT);
    gpio_set_dir(OUT_ENABLE_8__15, GPIO_OUT);

    int notDataWrite = WRITE_DATA_0__7;
    int dataWrite = WRITE_DATA_8__15;
    int notOutEnable = OUT_ENABLE_0__7;
    int outEnable = OUT_ENABLE_8__15;
    int readData = READ_DATA_0__7;
    int notReadData = READ_DATA_8__15;
    if (iSide == 1)
    {
        notDataWrite = WRITE_DATA_8__15;
        dataWrite = WRITE_DATA_0__7;
        notOutEnable = OUT_ENABLE_8__15;
        outEnable = OUT_ENABLE_0__7;
        readData = READ_DATA_8__15;
        notReadData = READ_DATA_0__7;
    }


    gpio_set_dir(DATA_0, GPIO_OUT);
    gpio_set_dir(DATA_1, GPIO_OUT);
    gpio_set_dir(DATA_2, GPIO_OUT);
    gpio_set_dir(DATA_3, GPIO_OUT);
    gpio_set_dir(DATA_4, GPIO_OUT);
    gpio_set_dir(DATA_5, GPIO_OUT);
    gpio_set_dir(DATA_6, GPIO_OUT);
    gpio_set_dir(DATA_7, GPIO_OUT);

    gpio_put(DATA_0, true);
    gpio_put(DATA_1, true);
    gpio_put(DATA_2, true);
    gpio_put(DATA_3, true);
    gpio_put(DATA_4, true);
    gpio_put(DATA_5, true);
    gpio_put(DATA_6, true);
    gpio_put(DATA_7, true);
    gpio_put(notDataWrite, true);  //Latch output enables into pre-register
    gpio_put(notDataWrite, false);
    gpio_put(DATA_0, false);
    gpio_put(DATA_1, false);
    gpio_put(DATA_2, false);
    gpio_put(DATA_3, false);
    gpio_put(DATA_4, false);
    gpio_put(DATA_5, false);
    gpio_put(DATA_6, false);
    gpio_put(DATA_7, false);
    gpio_put(notOutEnable, true);  //Latch output enables into pre-register
    gpio_put(notOutEnable, false);

    gpio_put(DATA_0, true);
    gpio_put(DATA_1, true);
    gpio_put(DATA_2, true);
    gpio_put(DATA_3, true);
    gpio_put(DATA_4, true);
    gpio_put(DATA_5, true);
    gpio_put(DATA_6, true);
    gpio_put(DATA_7, true);
    gpio_put(dataWrite, true);
    gpio_put(dataWrite, false);
    gpio_put(DATA_0, true);
    gpio_put(DATA_1, true);
    gpio_put(DATA_2, true);
    gpio_put(DATA_3, true);
    gpio_put(DATA_4, true);
    gpio_put(DATA_5, true);
    gpio_put(DATA_6, true);
    gpio_put(DATA_7, true);
    gpio_put(outEnable, true);  //Latch output enables 8..15 into pre-register
    gpio_put(outEnable, false);

    gpio_put(WRITE_OUT, true);  //Pulse write line to Write outputs enable pre-register into outputs enable register; disabling outputs 8..15 but enabling outputs 0..7.  This will also write the ouput values with whatever is currently in the output values register.
    gpio_put(WRITE_OUT, false);

    bool b = false;
    for (;;)
    {
        gpio_put(READ_IN, true);
        sleep_us(0);
        gpio_put(READ_IN, false);

        gpio_set_dir(DATA_0, GPIO_IN);
        gpio_set_dir(DATA_1, GPIO_IN);
        gpio_set_dir(DATA_2, GPIO_IN);
        gpio_set_dir(DATA_3, GPIO_IN);
        gpio_set_dir(DATA_4, GPIO_IN);
        gpio_set_dir(DATA_5, GPIO_IN);
        gpio_set_dir(DATA_6, GPIO_IN);
        gpio_set_dir(DATA_7, GPIO_IN);

        gpio_put(readData, true);
        sleep_us(0);
        bool gpio0 = gpio_get(DATA_0);
        bool gpio1 = gpio_get(DATA_1);
        bool gpio2 = gpio_get(DATA_2);
        bool gpio3 = gpio_get(DATA_3);
        bool gpio4 = gpio_get(DATA_4);
        bool gpio5 = gpio_get(DATA_5);
        bool gpio6 = gpio_get(DATA_6);
        bool gpio7 = gpio_get(DATA_7);
        gpio_put(readData, false);
        gpio_put(25, gpio0);

        gpio_set_dir(DATA_0, GPIO_OUT);
        gpio_set_dir(DATA_1, GPIO_OUT);
        gpio_set_dir(DATA_2, GPIO_OUT);
        gpio_set_dir(DATA_3, GPIO_OUT);
        gpio_set_dir(DATA_4, GPIO_OUT);
        gpio_set_dir(DATA_5, GPIO_OUT);
        gpio_set_dir(DATA_6, GPIO_OUT);
        gpio_set_dir(DATA_7, GPIO_OUT);

        gpio_put(DATA_0, gpio7);
        gpio_put(DATA_1, gpio6);
        gpio_put(DATA_2, gpio5);
        gpio_put(DATA_3, gpio4);
        gpio_put(DATA_4, gpio3);
        gpio_put(DATA_5, gpio2);
        gpio_put(DATA_6, gpio1);
        gpio_put(DATA_7, gpio0);
        gpio_put(dataWrite, true);
        gpio_put(dataWrite, false);

        gpio_put(WRITE_OUT, true);
        gpio_put(WRITE_OUT, false);

        sleep_ms(50);
        b = !b;
    }
}

int main() 
{
    doGPIO(0);
}


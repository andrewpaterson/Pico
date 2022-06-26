#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/binary_info.h"
#include "LCDDisplay.h"


const uint PIN_LED_PI = 25;
const uint PIN_LED0 = 2;
const uint PIN_LED1 = 1;
const uint PIN_LED2 = 0;

const uint PIN_CS0 = 15;
const uint PIN_CS1 = 27;
const uint PIN_CS2 = 28;

const uint PIN_LE = 14;

const uint PIN_L0 = 26;
const uint PIN_L1 = 22;
const uint PIN_L2 = 21;
const uint PIN_L3 = 20;
const uint PIN_L4 = 19;
const uint PIN_L5 = 18;
const uint PIN_L6 = 17;
const uint PIN_L7 = 16;


uint make_latch_mask(uint val)
{
    uint v0 = val & 0x01 ? (1 << PIN_L0) : 0;
    uint v1 = val & 0x02 ? (1 << PIN_L1) : 0;
    uint v2 = val & 0x04 ? (1 << PIN_L2) : 0;
    uint v3 = val & 0x08 ? (1 << PIN_L3) : 0;
    uint v4 = val & 0x10 ? (1 << PIN_L4) : 0;
    uint v5 = val & 0x20 ? (1 << PIN_L5) : 0;
    uint v6 = val & 0x40 ? (1 << PIN_L6) : 0;
    uint v7 = val & 0x80 ? (1 << PIN_L7) : 0;

    uint mask = v0 | v1 | v2 | v3 | v4 | v5 | v6 | v7;
    return mask;
}

uint make_chip_select_mask(uint val)
{
    uint v0 = val & 0x01 ? (1 << PIN_CS0) : 0;
    uint v1 = val & 0x02 ? (1 << PIN_CS1) : 0;
    uint v2 = val & 0x04 ? (1 << PIN_CS2) : 0;

    uint mask = v0 | v1 | v2;
    return mask;
}

uint make_led_mask(uint val)
{
    uint v0 = val & 0x01 ? (1 << PIN_LED0) : 0;
    uint v1 = val & 0x02 ? (1 << PIN_LED1) : 0;
    uint v2 = val & 0x04 ? (1 << PIN_LED2) : 0;

    uint mask = v0 | v1 | v2;
    return mask;
}


int main() 
{
    bi_decl(bi_program_description("LCD Display."));

    stdio_init_all();

    init_lcd();

    uint latchMask = make_latch_mask(0xff);
    gpio_init_mask(latchMask);
    gpio_set_dir_in_masked(latchMask);

    uint chipSelectMask = make_chip_select_mask(0x7);
    gpio_init_mask(chipSelectMask);
    gpio_set_dir_out_masked(chipSelectMask);
    gpio_put_masked(chipSelectMask, 0);

    gpio_init(PIN_LE);
    gpio_set_dir(PIN_LE, true);
    gpio_put(PIN_LE, false);

    uint ledMask = make_led_mask(0x7);
    gpio_init_mask(ledMask);
    gpio_set_dir_out_masked(ledMask);
    gpio_put_masked(ledMask, 0);

    bool led = true;
    gpio_init(PIN_LED_PI);
    gpio_set_dir(PIN_LED_PI, true);

    char szLine1[16];
    char szLine2[16];
    char szDecimal[16];

    uint chipTested = 0xff;
    while(true)
    {
        gpio_put(PIN_LED_PI, led);
        
        gpio_put(PIN_LE, true);
        sleep_us_high_power(1);
        gpio_put(PIN_LE, false);

        strcpy(szLine1, " 00|01|02|03|04");
        szLine2[0] = ' ';
        szLine2[1] = '\0';
        for (int chip = 0; chip <= 4; chip++)
        {
            if ((chip == chipTested) || (chipTested == 0xff))
            {
                uint chipPins = make_chip_select_mask(chip);
                gpio_put_masked(chipSelectMask, chipPins);
                sleep_us_high_power(1);
                
                uint v0 = gpio_get(PIN_L0);
                uint v1 = gpio_get(PIN_L1) << 1;
                uint v2 = gpio_get(PIN_L2) << 2;
                uint v3 = gpio_get(PIN_L3) << 3;
                uint v4 = gpio_get(PIN_L4) << 4;
                uint v5 = gpio_get(PIN_L5) << 5;
                uint v6 = gpio_get(PIN_L6) << 6;
                uint v7 = gpio_get(PIN_L7) << 7;

                uint val = v0 | v1 | v2 | v3 | v4 | v5 | v6 | v7;

                if (val < 16)
                {
                    strcat(szLine2, "0");
                    strcat(szLine2, itoa(val, szDecimal, 16));
                }
                else
                {
                    strcat(szLine2, itoa(val, szDecimal, 16));
                }
            }
            else
            {
                strcat(szLine2, "  ");
            }

            if (chip != 4)
            {
                strcat(szLine2, "|");
            }
        }

        put_clear();
        put_lines(szLine1, szLine2);
        sleep_us_high_power(10000);

        led = !led;
    }
}


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "LCDDisplay.h"
#include "ShiftRegister.h"
#include "GeneralPins.h"


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
    stdio_init_all();

    S595OutPins sShiftOut;
    sShiftOut.Init(PIN_NOT_SET, 18, 17, 16, PIN_NOT_SET);
    init_shift(&sShiftOut);

    S11BitLCDPins s11BitPins;

    s11BitPins.Init(13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3);
    init_lcd(&s11BitPins);
    gpio_init(17);
    gpio_set_dir(17, true);

    gpio_init(18);
    gpio_set_dir(18, true);

    gpio_init(19);
    gpio_set_dir(19, true);

    bool led = true;
    gpio_init(PIN_LED_PI);
    gpio_set_dir(PIN_LED_PI, true);

    char szLine1[16];
    char szLine2[16];
    char szDecimal[16];

    int iValue = 1;
    while(true)
    {
        gpio_put(PIN_LED_PI, led);
        
        // gpio_put(PIN_LE, true);
        // sleep_us_high_power(1);
        // gpio_put(PIN_LE, false);

        // gpio_put_masked(ledMask, led ? 7 : 0);

        // strcpy(szLine1, " 00|01|02|03|04");
        // szLine2[0] = ' ';
        // szLine2[1] = '\0';
        // for (int chip = 0; chip <= 4; chip++)
        // {
        //     if ((chip == chipTested) || (chipTested == 0xff))
        //     {
        //         uint chipPins = make_chip_select_mask(chip);
        //         gpio_put_masked(chipSelectMask, chipPins);
        //         sleep_us_high_power(1);
                
        //         uint v0 = gpio_get(PIN_L0);
        //         uint v1 = gpio_get(PIN_L1) << 1;
        //         uint v2 = gpio_get(PIN_L2) << 2;
        //         uint v3 = gpio_get(PIN_L3) << 3;
        //         uint v4 = gpio_get(PIN_L4) << 4;
        //         uint v5 = gpio_get(PIN_L5) << 5;
        //         uint v6 = gpio_get(PIN_L6) << 6;
        //         uint v7 = gpio_get(PIN_L7) << 7;

        //         uint val = (v0 | v1 | v2 | v3 | v4 | v5 | v6 | v7); // | (rand() & 255);

        //         if (val < 16)
        //         {
        //             strcat(szLine2, "0");
        //             strcat(szLine2, itoa(val, szDecimal, 16));
        //         }
        //         else if (val < 256)
        //         {
        //             strcat(szLine2, itoa(val, szDecimal, 16));
        //         }
        //         else
        //         {
        //             strcat(szLine2, "XX");
        //         }
        //     }
        //     else
        //     {
        //         strcat(szLine2, "  ");
        //     }

        //     if (chip != 4)
        //     {
        //         strcat(szLine2, "|");
        //     }
        // }

        // put_clear(&s11BitPins);
        // put_lines(&s11BitPins, szLine1, szLine2);

        shift_out(&sShiftOut, (uint16_t)iValue);
        iValue <<= 1;
        if (iValue == 0x10000)
        {
            iValue = 1;
        }

        sleep_us_high_power(20000);

        led = !led;
    }
}


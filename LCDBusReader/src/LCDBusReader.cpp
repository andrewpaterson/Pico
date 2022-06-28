#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "LCDDisplay.h"
#include "ShiftRegister.h"
#include "ShiftLCDDisplay.h"
#include "GeneralPins.h"


void shift_LCD(void)
{
    S595OutPins sShiftOut;
    sShiftOut.Init(PIN_NOT_SET, 18, 17, 16, PIN_NOT_SET);
    init_shift(&sShiftOut);

    init_lcd(&sShiftOut);
}


void parallel_LCD(void)
{
    bool led = true;
    char szLine1[16];
    char szLine2[16];

    S11BitLCDPins   sPins;

    sPins.Init(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

    init_lcd(&sPins);
    put_clear(&sPins);
    put_to_address(&sPins, "A", 0);

    // int     i = 0;
    // while(true)
    // {
    //     strcpy(szLine1, " 00|01|02|03|04");
    //     itoa(i, szLine2, 10);

    //     put_clear(&sPins);
    //     put_lines(&sPins, szLine1, szLine2);

    //     sleep_us_high_power(200000);

    //     i++;

    //     gpio_put(25, led);
    //     led = !led;
    // }
}


int main() 
{
    stdio_init_all();
    gpio_init(25);
    gpio_set_dir(25, true);

    parallel_LCD();
}


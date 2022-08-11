#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "LCDDisplay.h"
#include "ShiftRegister.h"
#include "ShiftLCDDisplay.h"
#include "HEXDisplay.h"
#include "GeneralPins.h"
#include "UARTComm.h"


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


void shift_LCD(void)
{
    S595OutPins sPins;
    sPins.Init(PIN_NOT_SET, 18, 17, 16, PIN_NOT_SET);
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


void parallel_LCD(void)
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

int     giDelay;

int main() 
{
    stdio_init_all();
    gpio_init(25);
    gpio_set_dir(25, true);

    gpio_init(2);
    gpio_set_dir(2, false);

    uart_inst_t* pUart = init_uart(0, 1, 115200 * 4);
    int iIRQ = get_uart_irq(pUart);

    irq_set_exclusive_handler(iIRQ, uart_receive_irq);
    irq_set_enabled(iIRQ, true);
    uart_set_irq_enables(pUart, true, false);

    bool bSlave = gpio_get(2);
    if (bSlave)
    {
        giDelay = 1000000;
    }
    else
    {
        giDelay = 400000;
    }

    //parallel_LCD();
    if (!bSlave)
    {
        bool bLed = true;

        SDualHexDigitPins   sDigits;
        sDigits.Init(PIN_NOT_SET, PIN_NOT_SET, 13, 12, 11, 10, 9, 8, 7, 6);
        init_dual_hex(&sDigits);

        S165InPins sShiftIn;
        sShiftIn.Init(26, 27, 28, true, true, true);
        init_shift(&sShiftIn);

        for (;;)
        {
            gpio_put(25, bLed);
            if (bLed)
            {
                uart_puts(pUart, "Fast\n");
            }
            else
            {
                uart_puts(pUart, "Slow\n");
            }
            
            int delay = giDelay;

            uint64_t start = time_us_64();
            uint64_t expectedEnd = start + delay;
            uint64_t end = start;

            while (expectedEnd > end)
            {
                end = time_us_64();
                
                uint16_t uiValue = shift_in(&sShiftIn);
                int iButton = get_snes_button(uiValue);

                set_dual_hex_value(&sDigits, iButton);

                sleep_us_high_power(30000);
            }            
            bLed = !bLed;
        }
    }
    else
    {
        bool bLed = true;
        for (;;)
        {
            gpio_put(25, bLed);

            int delay = giDelay;

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
                    giDelay = 20000;
                    if (memcmp(szMessage, "Fast", 4) == 0)
                    {
                        giDelay = 20000;
                    }
                    else if (memcmp(szMessage, "Slow", 4) == 0)
                    {
                        giDelay = 50000;
                    }
                }
                sleep_us_high_power(1000);
            }            
            bLed = !bLed;
        }
    }
}


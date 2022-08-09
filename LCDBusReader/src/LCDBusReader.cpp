#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "LCDDisplay.h"
#include "ShiftRegister.h"
#include "ShiftLCDDisplay.h"
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

int giDelay;

void on_uart_rx() 
{
    while (uart_is_readable(uart0)) 
    {
        uint8_t ch = uart_getc(uart0);
        if (ch == 1)
        {
            giDelay = 20000;
        }
        else if (ch == 2)
        {
            giDelay = 50000;
        }
    }
}

int main() 
{
    stdio_init_all();
    gpio_init(25);
    gpio_set_dir(25, true);

    gpio_init(2);
    gpio_set_dir(2, false);

    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);
    
    uart_init(uart0, 2400);
    uart_set_baudrate(uart0, 115200);
    uart_set_hw_flow(uart0, false, false);
    uart_set_format(uart0, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(uart0, false);

    bool bReceiver = gpio_get(2);
    if (bReceiver)
    {
        giDelay = 2000000;

        irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
        irq_set_enabled(UART0_IRQ, true);
        uart_set_irq_enables(uart0, true, false);
    }
    else
    {
        giDelay = 2000000;
    }

    //parallel_LCD();
    for (;;)
    {
        gpio_put(25, true);
        if (!bReceiver)        
        {
            uart_putc(uart0, 1);
        }        
        sleep_us_high_power(giDelay);
        gpio_put(25, false);
        if (!bReceiver)        
        {
            uart_putc(uart0, 2);
        }        
        sleep_us_high_power(giDelay);
    }
}


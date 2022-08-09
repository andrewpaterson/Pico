#include <string.h>
#include "UARTComm.h"

uart_inst_t* init_uart(int iTxPin, int iRxPin, int iBaudRate)
{
    uart_inst_t* pUart = NULL;
    if ((iTxPin == 0) && (iRxPin == 1))
    {
        pUart = uart0;
    }

    if (pUart)
    {
        gpio_set_function(iTxPin, GPIO_FUNC_UART);
        gpio_set_function(iRxPin, GPIO_FUNC_UART);

        uart_init(pUart, 2400);
        uart_set_baudrate(pUart, iBaudRate);
        uart_set_hw_flow(pUart, false, false);
        uart_set_format(pUart, 8, 1, UART_PARITY_NONE);
        uart_set_fifo_enabled(pUart, false);
    }
    return pUart;    
}

int get_uart_irq(uart_inst_t* pUart)
{
    if (pUart == uart0)
    {
        return UART0_IRQ;
    }
    else
    {
        return -1;
    }
}

char    gszMessage[10][256];
int     giCurrentMessage = 0;
int     giValidMessage = -1;
int     giMessageCharIndex = 0;

void uart_receive_irq(void) 
{
    while (uart_is_readable(uart0)) 
    {
        uint8_t c = uart_getc(uart0);
        gszMessage[giCurrentMessage][giMessageCharIndex] = c;

        if (c == '\n')
        {
            giMessageCharIndex = 0;
            giValidMessage = giCurrentMessage;
            giCurrentMessage++;
            if (giCurrentMessage == 10)
            {
                giCurrentMessage = 0;
            }
        }
        else
        {
            giMessageCharIndex++;
            if (giMessageCharIndex == 256)
            {
                giMessageCharIndex = 255;
            }
        }
    }
}

bool read_uart_message(char* szDest)
{
    if (giValidMessage != -1)
    {
        strcpy(szDest, gszMessage[giValidMessage]);
        giValidMessage = -1;
        return true;
    }
    return false;
}


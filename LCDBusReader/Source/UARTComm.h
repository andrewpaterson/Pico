#ifndef __UART_COMM_H__
#define __UART_COMM_H__
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

uart_inst_t* init_uart(int pin1, int pin2, int iBaudRate);
int get_uart_irq(uart_inst_t* pUart);
void uart_receive_irq(void);
bool read_uart_message(char* szDest);
bool write_uart_message(uart_inst_t* pUart, const char* szMessage);

#endif // __UART_COMM_H__


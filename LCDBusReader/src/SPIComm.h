#ifndef __SPI_COMM_H__
#define __SPI_COMM_H__
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

struct SSPIPins
{
    int pinClk;
    int pinTx;
    int pinRx;
    int pinEnable;
    bool bEnableHigh;

    spi_inst_t *pSPI;

    void Init(int pinClk, int pinTx, int pinRx, int pinEnable, bool bEnableHigh);
};


void init_spi(SSPIPins* psPins);
void write_spi( SSPIPins* psPins, uint16_t data);


#endif // __SPI_COMM_H__


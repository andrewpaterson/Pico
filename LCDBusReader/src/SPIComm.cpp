#include "SPIComm.h"


void SSPIPins::Init(int pinClk, int pinTx, int pinRx, int pinEnable, bool bEnableHigh)
{
    this->pinClk = pinClk;
    this->pinTx = pinTx;
    this->pinRx = pinRx;
    this->pinEnable = pinEnable;
    this->bEnableHigh = bEnableHigh;

    if (pinClk == 18)
    {
        pSPI = spi0;
    }
    else
    {
        pSPI = NULL;
    }
}


void init_spi(SSPIPins* psPins)
{
    gpio_set_function(psPins->pinClk, GPIO_FUNC_SPI);
    gpio_set_function(psPins->pinTx, GPIO_FUNC_SPI);
    gpio_set_function(psPins->pinRx, GPIO_FUNC_SPI);
    gpio_init(psPins->pinEnable);
    gpio_set_dir(psPins->pinEnable, GPIO_OUT);

    spi_init(psPins->pSPI, 100000);

    spi_set_format( psPins->pSPI,
                    8,    
                    SPI_CPOL_1,    
                    SPI_CPHA_1,    
                    SPI_MSB_FIRST);
}

void write_spi( SSPIPins* psPins, uint16_t uiData)
{
    uint8_t auiData[2];
    auiData[0] = uiData >> 8;
    auiData[1] = uiData;
    gpio_put(psPins->pinEnable, psPins->bEnableHigh);
    spi_write_blocking(psPins->pSPI, auiData, 2);
    gpio_put(psPins->pinEnable, !psPins->bEnableHigh);
}


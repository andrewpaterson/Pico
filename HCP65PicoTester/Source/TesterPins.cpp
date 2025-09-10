#include "TesterPins.h"


uint8 aiDataPins[] = { DATA_LINE_7, DATA_LINE_6, DATA_LINE_5, DATA_LINE_4, DATA_LINE_3, DATA_LINE_2, DATA_LINE_1, DATA_LINE_0 };

uint8 aiAddressPins[] = { ADDR_LINE_0, ADDR_LINE_1, ADDR_LINE_2, ADDR_LINE_3, ADDR_LINE_4, ADDR_LINE_5 };

uint8 aiReadAddresses[16] = { ADDRESS_READ_GPIO__0__7, ADDRESS_READ_GPIO__8_15, ADDRESS_READ_GPIO_16_23, ADDRESS_READ_GPIO_24_31, ADDRESS_READ_GPIO_32_39,  ADDRESS_READ_GPIO_40_47,   ADDRESS_READ_GPIO_48_55,   ADDRESS_READ_GPIO_56_63,
                            ADDRESS_READ_GPIO_64_71, ADDRESS_READ_GPIO_72_79, ADDRESS_READ_GPIO_80_87, ADDRESS_READ_GPIO_88_95, ADDRESS_READ_GPIO_96_103, ADDRESS_READ_GPIO_104_111, ADDRESS_READ_GPIO_112_119, ADDRESS_READ_GPIO_120_127 };

void InitPicoPins(void)
{
    gpio_put_all(0);
    stdio_init_all();

    gpio_init(ONBOARD_LED);
    gpio_set_dir(ONBOARD_LED, GPIO_OUT);

    gpio_init(PWR_12V);
    gpio_init(PWR_5V_A);
    gpio_init(PWR_5V_B);
    gpio_init(PWR_5V_C);
    gpio_init(PWR_GND_A);
    gpio_init(PWR_GND_B);
    gpio_init(PWR_GND_C);
    gpio_init(PWR_GND_D);
    gpio_set_dir(PWR_12V, GPIO_OUT);
    gpio_set_dir(PWR_5V_A, GPIO_OUT);    
    gpio_set_dir(PWR_5V_B, GPIO_OUT);
    gpio_set_dir(PWR_5V_C, GPIO_OUT);    
    gpio_set_dir(PWR_GND_A, GPIO_OUT);
    gpio_set_dir(PWR_GND_B, GPIO_OUT);    
    gpio_set_dir(PWR_GND_C, GPIO_OUT);
    gpio_set_dir(PWR_GND_D, GPIO_OUT);

    gpio_put(PWR_5V_A, true);
    gpio_put(PWR_5V_A, false);
    gpio_put(PWR_5V_A, true);

    gpio_init(ENABLE_GPIO);
    gpio_init(ADDR_LINE_ENABLE);
    gpio_init(ADDR_LINE_0);
    gpio_init(ADDR_LINE_1);
    gpio_init(ADDR_LINE_2);
    gpio_init(ADDR_LINE_3);
    gpio_init(ADDR_LINE_4);
    gpio_init(ADDR_LINE_5);
    gpio_init(READ_IN);
    gpio_init(WRITE_OUT);
    gpio_set_dir(ENABLE_GPIO, GPIO_OUT);
    gpio_set_dir(ADDR_LINE_ENABLE, GPIO_OUT);
    gpio_set_dir(ADDR_LINE_0, GPIO_OUT);
    gpio_set_dir(ADDR_LINE_1, GPIO_OUT);
    gpio_set_dir(ADDR_LINE_2, GPIO_OUT);
    gpio_set_dir(ADDR_LINE_3, GPIO_OUT);
    gpio_set_dir(ADDR_LINE_4, GPIO_OUT);
    gpio_set_dir(ADDR_LINE_5, GPIO_OUT);
    gpio_set_dir(READ_IN, GPIO_OUT);
    gpio_set_dir(WRITE_OUT, GPIO_OUT);

    gpio_init(DATA_LINE_0);
    gpio_init(DATA_LINE_1);
    gpio_init(DATA_LINE_2);
    gpio_init(DATA_LINE_3);
    gpio_init(DATA_LINE_4);
    gpio_init(DATA_LINE_5);
    gpio_init(DATA_LINE_6);
    gpio_init(DATA_LINE_7);
    gpio_set_dir(DATA_LINE_0, GPIO_IN);
    gpio_set_dir(DATA_LINE_1, GPIO_IN);
    gpio_set_dir(DATA_LINE_2, GPIO_IN);
    gpio_set_dir(DATA_LINE_3, GPIO_IN);
    gpio_set_dir(DATA_LINE_4, GPIO_IN);
    gpio_set_dir(DATA_LINE_5, GPIO_IN);
    gpio_set_dir(DATA_LINE_6, GPIO_IN);
    gpio_set_dir(DATA_LINE_7, GPIO_IN);
}


bool gbDataRead = true;


void SetDataToInput(void)
{
    if (!gbDataRead)
    {
        gpio_set_dir_in_masked( 1 << DATA_LINE_0 | 
                                1 << DATA_LINE_1 |
                                1 << DATA_LINE_2 |
                                1 << DATA_LINE_3 |
                                1 << DATA_LINE_4 |
                                1 << DATA_LINE_5 |
                                1 << DATA_LINE_6 |
                                1 << DATA_LINE_7 );
        gbDataRead = true;
    }
}


void SetDataToOutput(void)
{
    if (gbDataRead)
    {
        gpio_set_dir_out_masked(1 << DATA_LINE_0 | 
                                1 << DATA_LINE_1 |
                                1 << DATA_LINE_2 |
                                1 << DATA_LINE_3 |
                                1 << DATA_LINE_4 |
                                1 << DATA_LINE_5 |
                                1 << DATA_LINE_6 |
                                1 << DATA_LINE_7 );
        gbDataRead = false;                    
    }
}


uint32 MakeAddress(uint8 uiAddress, bool bEnable)
{
    uint32 uiAddressOnPins =    (uiAddress & 0x01 ? (1ul << aiAddressPins[0]) : 0) | 
                                (uiAddress & 0x02 ? (1ul << aiAddressPins[1]) : 0) | 
                                (uiAddress & 0x04 ? (1ul << aiAddressPins[2]) : 0) | 
                                (uiAddress & 0x08 ? (1ul << aiAddressPins[3]) : 0) | 
                                (uiAddress & 0x10 ? (1ul << aiAddressPins[4]) : 0) |
                                (bEnable ? (1ul << ADDR_LINE_ENABLE) : 0);
    return uiAddressOnPins;
}


uint32 guiWriteDataMask = ( 1 << DATA_LINE_0 | 
                            1 << DATA_LINE_1 |
                            1 << DATA_LINE_2 |
                            1 << DATA_LINE_3 |
                            1 << DATA_LINE_4 |
                            1 << DATA_LINE_5 |
                            1 << DATA_LINE_6 |
                            1 << DATA_LINE_7 |
                            1 << ADDR_LINE_ENABLE |
                            1 << ADDR_LINE_0 |
                            1 << ADDR_LINE_1 |
                            1 << ADDR_LINE_2 |
                            1 << ADDR_LINE_3 |
                            1 << ADDR_LINE_4 |
                            1 << ADDR_LINE_5 );

uint32 guiReadDataMask =   (1 << ADDR_LINE_ENABLE |
                            1 << ADDR_LINE_0 |
                            1 << ADDR_LINE_1 |
                            1 << ADDR_LINE_2 |
                            1 << ADDR_LINE_3 |
                            1 << ADDR_LINE_4 |
                            1 << ADDR_LINE_5 );


uint8 ReverseByte(uint8 uiByte) 
{
    return ((uiByte & 0x01) << 7) | 
           ((uiByte & 0x02) << 5) | 
           ((uiByte & 0x04) << 3) | 
           ((uiByte & 0x08) << 1) | 
           ((uiByte & 0x10) >> 1) | 
           ((uiByte & 0x20) >> 3) | 
           ((uiByte & 0x40) >> 5) | 
           ((uiByte & 0x80) >> 7);
}


void WriteData(uint8 uiAddress, uint8 uiData)
{
    uint32    uiDataOnPins;
    uint32    uiAddressOnPins;

    uiData = ReverseByte(uiData);
    uiDataOnPins = make_8bit_mask(aiDataPins, uiData);

    SetDataToOutput();
    uiAddressOnPins = MakeAddress(uiAddress, true);
    gpio_put_masked(guiWriteDataMask, uiAddressOnPins | uiDataOnPins);

    uiAddressOnPins = MakeAddress(uiAddress, false);
    gpio_put_masked(guiWriteDataMask, uiAddressOnPins | uiDataOnPins);
}


void PulseWriteData(void)
{
    gpio_put(WRITE_OUT, true);
    gpio_put(WRITE_OUT, false);
}


void PulseReadData(void)
{
    gpio_put(READ_IN, true);
    gpio_put(READ_IN, false);
}


uint8 ReadData(uint8 uiAddress)
{
    uint32  uiDataOnPins;
    uint32  uiAddressOnPins;
    uint8   uiValue;

    SetDataToInput();

    uiAddressOnPins = MakeAddress(uiAddress, true);
    gpio_put_masked(guiReadDataMask, uiAddressOnPins);
    for (size i = 0; i < 1; i++)
    {
        asm volatile("nop");
    }

    uiDataOnPins = gpio_get_all();
    uiAddressOnPins = MakeAddress(uiAddress, false);
    gpio_put_masked(guiReadDataMask, uiAddressOnPins);
    
    if (uiAddress & 1 == 1)
    {
        uiValue =   (uiDataOnPins & (1ul << aiDataPins[0]) ? 1 << 0 : 0) | 
                    (uiDataOnPins & (1ul << aiDataPins[1]) ? 1 << 1 : 0) | 
                    (uiDataOnPins & (1ul << aiDataPins[2]) ? 1 << 2 : 0) | 
                    (uiDataOnPins & (1ul << aiDataPins[3]) ? 1 << 3 : 0) | 
                    (uiDataOnPins & (1ul << aiDataPins[4]) ? 1 << 4 : 0) | 
                    (uiDataOnPins & (1ul << aiDataPins[5]) ? 1 << 5 : 0) | 
                    (uiDataOnPins & (1ul << aiDataPins[6]) ? 1 << 6 : 0) | 
                    (uiDataOnPins & (1ul << aiDataPins[7]) ? 1 << 7 : 0);

    }
    else
    {
        uiValue =   (uiDataOnPins & (1ul << aiDataPins[0]) ? 1 << 0 : 0) | 
                    (uiDataOnPins & (1ul << aiDataPins[1]) ? 1 << 1 : 0) | 
                    (uiDataOnPins & (1ul << aiDataPins[2]) ? 1 << 2 : 0) | 
                    (uiDataOnPins & (1ul << aiDataPins[3]) ? 1 << 3 : 0) | 
                    (uiDataOnPins & (1ul << aiDataPins[4]) ? 1 << 4 : 0) | 
                    (uiDataOnPins & (1ul << aiDataPins[5]) ? 1 << 5 : 0) | 
                    (uiDataOnPins & (1ul << aiDataPins[6]) ? 1 << 6 : 0) | 
                    (uiDataOnPins & (1ul << aiDataPins[7]) ? 1 << 7 : 0);
    }

    return uiValue;
}


void SetAllToOutput(uint8 uiValue)
{
    for (size j = ADDRESS_OUTPUT_GPIO_56_63; j <= ADDRESS_OUTPUT_GPIO__0__7; j++)
    {
        WriteData(j, uiValue);
    }
}


void SetAllToData(uint8 uiValue)
{
    for (size j = ADDRESS_WRITE_GPIO_56_63; j <= ADDRESS_WRITE_GPIO__0__7; j++)
    {
        WriteData(j, uiValue);
    }
}


#include "TesterPins.h"
#include "TestFunctions.h"


void TestRead(bool bReadLeft)
{
    uint8    uiNoOutput0 = bReadLeft ? ADDRESS_OUTPUT_GPIO__0__7 : ADDRESS_OUTPUT_GPIO_32_39;
    uint8    uiNoOutput1 = bReadLeft ? ADDRESS_OUTPUT_GPIO__8_15 : ADDRESS_OUTPUT_GPIO_40_47;
    uint8    uiNoOutput2 = bReadLeft ? ADDRESS_OUTPUT_GPIO_16_23 : ADDRESS_OUTPUT_GPIO_48_55;
    uint8    uiNoOutput3 = bReadLeft ? ADDRESS_OUTPUT_GPIO_24_31 : ADDRESS_OUTPUT_GPIO_56_63;
    uint8    uiOutput0 = bReadLeft ? ADDRESS_OUTPUT_GPIO_32_39 : ADDRESS_OUTPUT_GPIO__0__7;
    uint8    uiOutput1 = bReadLeft ? ADDRESS_OUTPUT_GPIO_40_47 : ADDRESS_OUTPUT_GPIO__8_15;
    uint8    uiOutput2 = bReadLeft ? ADDRESS_OUTPUT_GPIO_48_55 : ADDRESS_OUTPUT_GPIO_16_23;
    uint8    uiOutput3 = bReadLeft ? ADDRESS_OUTPUT_GPIO_56_63 : ADDRESS_OUTPUT_GPIO_24_31;

    uint8    uiRead0 = bReadLeft ? ADDRESS_READ_GPIO__0__7 : ADDRESS_READ_GPIO_32_39;
    uint8    uiRead1 = bReadLeft ? ADDRESS_READ_GPIO__8_15 : ADDRESS_READ_GPIO_40_47;
    uint8    uiRead2 = bReadLeft ? ADDRESS_READ_GPIO_16_23 : ADDRESS_READ_GPIO_48_55;
    uint8    uiRead3 = bReadLeft ? ADDRESS_READ_GPIO_24_31 : ADDRESS_READ_GPIO_56_63;

    uint8    uiWrite0 = bReadLeft ? ADDRESS_WRITE_GPIO_32_39 : ADDRESS_WRITE_GPIO__0__7;
    uint8    uiWrite1 = bReadLeft ? ADDRESS_WRITE_GPIO_40_47 : ADDRESS_WRITE_GPIO__8_15;
    uint8    uiWrite2 = bReadLeft ? ADDRESS_WRITE_GPIO_48_55 : ADDRESS_WRITE_GPIO_16_23;
    uint8    uiWrite3 = bReadLeft ? ADDRESS_WRITE_GPIO_56_63 : ADDRESS_WRITE_GPIO_24_31;

    SetAllToData(0x00);

    WriteData(uiNoOutput0, 0x00);
    WriteData(uiNoOutput1, 0x00);
    WriteData(uiNoOutput2, 0x00);
    WriteData(uiNoOutput3, 0x00);
    WriteData(uiOutput0, 0xff);
    WriteData(uiOutput1, 0xff);
    WriteData(uiOutput2, 0Xff);
    WriteData(uiOutput3, 0xff);
    PulseWriteData();

    uint8    uiValue;
    for (;;)
    {
        PulseReadData();
        uiValue = ReadData(uiRead0);
        WriteData(uiWrite0, uiValue);

        uiValue = ReadData(uiRead1);
        WriteData(uiWrite1, uiValue);

        uiValue = ReadData(uiRead2);
        WriteData(uiWrite2, uiValue);

        uiValue = ReadData(uiRead3);
        WriteData(uiWrite3, uiValue);
        PulseWriteData();
    }
}


void TestWrite(void)
{
    uint32 iDataOffset;
    uint32 iAddress;

    for (;;)
    {
        SetAllToOutput(0xff);
        PulseWriteData();
        for (size i = 0; i < 64; i++)
        {
            SetAllToData(0x00);

            iAddress = i / 8;
            iDataOffset = 1 << (7 - (i % 8));
            WriteData(ADDRESS_WRITE_GPIO__0__7 - iAddress, iDataOffset);
            PulseWriteData();
            sleep_ms(40);
        }

        SetAllToData(0xff);
        SetAllToOutput(0x00);
        PulseWriteData();
        for (size i = 63; i >= 0; i--)
        {
            SetAllToOutput(0x00);

            iAddress = i / 8;
            iDataOffset = 1 << (7 - (i % 8));
            WriteData(ADDRESS_OUTPUT_GPIO__0__7 - iAddress, iDataOffset);
            PulseWriteData();
            sleep_ms(40);
        }
    }
}


#include "LCDDisplay.h"
#include "hardware/gpio.h"
#include "GeneralPins.h"


void S11BitLCDPins::Init(uint uiRegisterSelectGPIO,
                         uint uiReadWriteBGPIO,
                         uint uiEnableGPIO,
                         uint uiData0GPIO,
                         uint uiData1GPIO,
                         uint uiData2GPIO,
                         uint uiData3GPIO,
                         uint uiData4GPIO,
                         uint uiData5GPIO,
                         uint uiData6GPIO,
                         uint uiData7GPIO)
{
    this->uiRegisterSelectGPIO = uiRegisterSelectGPIO;
    this->uiReadWriteBGPIO = uiReadWriteBGPIO;
    this->uiEnableGPIO = uiEnableGPIO;
    auiDataGPIO[0] = uiData0GPIO;
    auiDataGPIO[1] = uiData1GPIO;
    auiDataGPIO[2] = uiData2GPIO;
    auiDataGPIO[3] = uiData3GPIO;
    auiDataGPIO[4] = uiData4GPIO;
    auiDataGPIO[5] = uiData5GPIO;
    auiDataGPIO[6] = uiData6GPIO;
    auiDataGPIO[7] = uiData7GPIO;
}


const uint eCycleTime = 1;


uint make_lcd_mask(S11BitLCDPins* psPins, bool rs, bool rw, bool e, uint val);
uint make_command_mask(S11BitLCDPins* psPins, bool rs, bool rw, bool e);
void put_lines(S11BitLCDPins* psPins, uint writeMask, uint readMask, char* szLine1, char* szLine2);
void put_clear(S11BitLCDPins* psPins, uint writeMask, uint readMask);
void init_lcd(S11BitLCDPins* psPins, uint writeMask, uint commandMask);
void put_display_address(S11BitLCDPins* psPins, uint writeMask, uint readMask, uint address);
void put_string(S11BitLCDPins* psPins, uint writeMask, uint readMask, char* sz, int maxLength);


void put_lines(S11BitLCDPins* psPins, char* szLine1, char* szLine2)
{
    uint writeMask = make_lcd_mask(psPins, true, true, true, 0xff);
    uint commandMask = make_command_mask(psPins, true, true, true);
    put_lines(psPins, writeMask, commandMask, szLine1, szLine2);
}


void put_clear(S11BitLCDPins* psPins)
{
    uint writeMask = make_lcd_mask(psPins, true, true, true, 0xff);
    uint commandMask = make_command_mask(psPins, true, true, true);
    put_clear(psPins, writeMask, commandMask);
}


void init_lcd(S11BitLCDPins* psPins)
{
    uint writeMask = make_lcd_mask(psPins, true, true, true, 0xff);
    uint commandMask = make_command_mask(psPins, true, true, true);
    init_lcd(psPins, writeMask, commandMask);
}


void put_to_address(S11BitLCDPins* psPins, char* sz, uint uiAddress)
{
    uint writeMask = make_lcd_mask(psPins, true, true, true, 0xff);
    uint commandMask = make_command_mask(psPins, true, true, true);

    put_display_address(psPins, writeMask, commandMask, uiAddress);
    put_string(psPins, writeMask, commandMask, sz, 16);
}


uint make_command_mask(S11BitLCDPins* psPins, bool rs, bool rw, bool e)
{
    uint rs_s = rs ? (1 << psPins->uiRegisterSelectGPIO) : 0;
    uint rw_s = rw ? (1 << psPins->uiReadWriteBGPIO) : 0;
    uint e_s = e ? (1 << psPins->uiEnableGPIO) : 0;

    uint mask =  rs_s | rw_s | e_s;
    return mask;
}


uint make_lcd_mask(S11BitLCDPins* psPins, bool rs, bool rw, bool e, uint val)
{
    uint mask = make_command_mask(psPins, rs, rw, e);

    uint v0 = val & 0x01 ? (1 << psPins->auiDataGPIO[0]) : 0;
    uint v1 = val & 0x02 ? (1 << psPins->auiDataGPIO[1]) : 0;
    uint v2 = val & 0x04 ? (1 << psPins->auiDataGPIO[2]) : 0;
    uint v3 = val & 0x08 ? (1 << psPins->auiDataGPIO[3]) : 0;
    uint v4 = val & 0x10 ? (1 << psPins->auiDataGPIO[4]) : 0;
    uint v5 = val & 0x20 ? (1 << psPins->auiDataGPIO[5]) : 0;
    uint v6 = val & 0x40 ? (1 << psPins->auiDataGPIO[6]) : 0;
    uint v7 = val & 0x80 ? (1 << psPins->auiDataGPIO[7]) : 0;

    mask = mask | v0 | v1 | v2 | v3 | v4 | v5 | v6 | v7;
    return mask;
}


void put(S11BitLCDPins* psPins, uint writeMask, uint readMask, bool rs, bool rw, uint data, uint minDelay)
{
    uint eLow = make_lcd_mask(psPins, rs, rw, false, data);
    uint eHigh = make_lcd_mask(psPins, rs, rw, true, data);

    gpio_set_dir_out_masked(writeMask);

    gpio_put_masked(writeMask, eLow);
    sleep_us_high_power(eCycleTime);

    gpio_put_masked(writeMask, eHigh);
    sleep_us_high_power(eCycleTime);
    
    gpio_put_masked(writeMask, eLow);
    sleep_us_high_power(minDelay + eCycleTime);
    gpio_put_masked(writeMask, 0);
}


void put_function(S11BitLCDPins* psPins, uint writeMask, uint readMask, bool eightBit, bool twoLines, bool largeFont)
{
    uint function = 0b00100000;
    uint f2 = eightBit ?  0b00010000 : 0;
    uint f1 = twoLines ?  0b00001000 : 0;
    uint f0 = largeFont ? 0b00000100 : 0;
    
    function = function | f2 | f1 | f0;
    put(psPins, writeMask, readMask, false, false, function, 39);
}


void put_display(S11BitLCDPins* psPins, uint writeMask, uint readMask, bool displayOn, bool cursorOn, bool cursoBlink)
{
    uint display =  0b00001000;  
    uint d2 = displayOn ?  0b00000100 : 0;
    uint d1 = cursorOn ?   0b00000010 : 0;
    uint d0 = cursoBlink ? 0b00000001 : 0;
    
    display = display | d2 | d1 | d0;
    put(psPins, writeMask, readMask, false, false, display, 39);
}


void put_shift(S11BitLCDPins* psPins, uint writeMask, uint readMask, bool increment, bool shiftDisplay)
{
    uint entry =  0b00000100;
    uint e1 = increment ?   0b00000010 : 0;
    uint e0 = shiftDisplay ? 0b00000001 : 0;
    
    entry = entry | e1 | e0;
    put(psPins, writeMask, readMask, false, false, entry, 39);
}

void put_clear(S11BitLCDPins* psPins, uint writeMask, uint readMask)
{
    put(psPins, writeMask, readMask, false, false, 0b00000001, 1530);
}


void put_string(S11BitLCDPins* psPins, uint writeMask, uint readMask, char* sz, int maxLength)
{
    char*   pc = sz;
    int     length = 0;
    
    while (*pc && length < maxLength)
    {
        put(psPins, writeMask, readMask, true, false, *pc, 43);
        pc++;
        length++;
    }
}


void put_display_address(S11BitLCDPins* psPins, uint writeMask, uint readMask, uint address)
{
    uint ddAddress = 0b10000000 | address;
    
    put(psPins, writeMask, readMask, false, false, ddAddress, 39);
}


void put_lines(S11BitLCDPins* psPins, uint writeMask, uint readMask, char* szLine1, char* szLine2)
{
    put_display_address(psPins, writeMask, readMask, 0x00);
    put_string(psPins, writeMask, readMask, szLine1, 16);
    put_display_address(psPins, writeMask, readMask, 0x40);
    put_string(psPins, writeMask, readMask, szLine2, 16);
}

void init_lcd(S11BitLCDPins* psPins, uint writeMask, uint commandMask)
{
    gpio_init_mask(writeMask);
    gpio_set_dir_out_masked(writeMask);

    //Function must be 'put' 3 times to ensure that it is in the correct state.
    put_function(psPins, writeMask, commandMask, true, true, false);
    put_function(psPins, writeMask, commandMask, true, true, false);
    put_function(psPins, writeMask, commandMask, true, true, false);

    put_display(psPins, writeMask, commandMask, true, false, false);

    put_shift(psPins, writeMask, commandMask, true, false);
}


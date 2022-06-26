#include "LCDDisplay.h"
#include "hardware/gpio.h"


const uint PIN_REGISTER_SELECT = 13;
const uint PIN_READ_WRITEB = 12;
const uint PIN_ENABLE = 11;
const uint PIN_DB0 = 10;
const uint PIN_DB1 = 9;
const uint PIN_DB2 = 8;
const uint PIN_DB3 = 7;
const uint PIN_DB4 = 6;
const uint PIN_DB5 = 5;
const uint PIN_DB6 = 4;
const uint PIN_DB7 = 3;


const uint eCycleTime = 1;


uint make_lcd_mask(bool rs, bool rw, bool e, uint val);
uint make_command_mask(bool rs, bool rw, bool e);
void put_lines(uint writeMask, uint readMask, char* szLine1, char* szLine2);
void put_clear(uint writeMask, uint readMask);
void init_lcd(uint writeMask, uint commandMask);


void put_lines(char* szLine1, char* szLine2)
{
    uint writeMask = make_lcd_mask(true, true, true, 0xff);
    uint commandMask = make_command_mask(true, true, true);
    put_lines(writeMask, commandMask, szLine1, szLine2);
}


void put_clear(void)
{
    uint writeMask = make_lcd_mask(true, true, true, 0xff);
    uint commandMask = make_command_mask(true, true, true);
    put_clear(writeMask, commandMask);
}


void init_lcd(void)
{
    uint writeMask = make_lcd_mask(true, true, true, 0xff);
    uint commandMask = make_command_mask(true, true, true);
    init_lcd(writeMask, commandMask);
}


void sleep_us_high_power(uint delay)
{
    uint64_t start = time_us_64();
    uint64_t expectedEnd = start + delay;
    uint64_t end = start;

    while (expectedEnd > end)
    {
        end = time_us_64();
    }
}

uint make_command_mask(bool rs, bool rw, bool e)
{
    uint rs_s = rs ? (1 << PIN_REGISTER_SELECT) : 0;
    uint rw_s = rw ? (1 << PIN_READ_WRITEB) : 0;
    uint e_s = e ? (1 << PIN_ENABLE) : 0;

    uint mask =  rs_s | rw_s | e_s;
    return mask;
}

uint make_lcd_mask(bool rs, bool rw, bool e, uint val)
{
    uint mask = make_command_mask(rs, rw, e);

    uint v0 = val & 0x01 ? (1 << PIN_DB0) : 0;
    uint v1 = val & 0x02 ? (1 << PIN_DB1) : 0;
    uint v2 = val & 0x04 ? (1 << PIN_DB2) : 0;
    uint v3 = val & 0x08 ? (1 << PIN_DB3) : 0;
    uint v4 = val & 0x10 ? (1 << PIN_DB4) : 0;
    uint v5 = val & 0x20 ? (1 << PIN_DB5) : 0;
    uint v6 = val & 0x40 ? (1 << PIN_DB6) : 0;
    uint v7 = val & 0x80 ? (1 << PIN_DB7) : 0;

    mask = mask | v0 | v1 | v2 | v3 | v4 | v5 | v6 |v7;
    return mask;
}

void wait_for_busy(uint readMask)
{
    uint eLow = make_command_mask(0, 1, false);
    uint eHigh = make_command_mask(0, 1, true);

    gpio_set_dir_out_masked(readMask);
    gpio_set_dir(PIN_DB7, false);

    gpio_put_masked(readMask, eLow);
    sleep_us_high_power(eCycleTime);

    bool busy = true;
    while (busy)
    {
        gpio_put_masked(readMask, eHigh);
        sleep_us_high_power(eCycleTime);
        
        gpio_put_masked(readMask, eLow);
        busy = gpio_get(PIN_DB7);
        sleep_us_high_power(eCycleTime);
    }
}

void put(uint writeMask, uint readMask, bool rs, bool rw, uint data, uint minDelay)
{
    uint eLow = make_lcd_mask(rs, rw, false, data);
    uint eHigh = make_lcd_mask(rs, rw, true, data);

    gpio_set_dir_out_masked(writeMask);

    gpio_put_masked(writeMask, eLow);
    sleep_us_high_power(eCycleTime);

    gpio_put_masked(writeMask, eHigh);
    sleep_us_high_power(eCycleTime);
    
    gpio_put_masked(writeMask, eLow);
    sleep_us_high_power(minDelay + eCycleTime);

    wait_for_busy(readMask);
}


void put_function(uint writeMask, uint readMask, bool eightBit, bool twoLines, bool largeFont)
{
    uint function = 0b00100000;
    uint f2 = eightBit ?  0b00010000 : 0;
    uint f1 = twoLines ?  0b00001000 : 0;
    uint f0 = largeFont ? 0b00000100 : 0;
    
    function = function | f2 | f1 | f0;
    put(writeMask, readMask, false, false, function, 39);
}


void put_display(uint writeMask, uint readMask, bool displayOn, bool cursorOn, bool cursoBlink)
{
    uint display =  0b00001000;  
    uint d2 = displayOn ?  0b00000100 : 0;
    uint d1 = cursorOn ?   0b00000010 : 0;
    uint d0 = cursoBlink ? 0b00000001 : 0;
    
    display = display | d2 | d1 | d0;
    put(writeMask, readMask, false, false, display, 39);
}


void put_shift(uint writeMask, uint readMask, bool increment, bool shiftDisplay)
{
    uint entry =  0b00000100;
    uint e1 = increment ?   0b00000010 : 0;
    uint e0 = shiftDisplay ? 0b00000001 : 0;
    
    entry = entry | e1 | e0;
    put(writeMask, readMask, false, false, entry, 39);
}

void put_clear(uint writeMask, uint readMask)
{
    put(writeMask, readMask, false, false, 0b00000001, 1530);
}


void put_string(uint writeMask, uint readMask, char* sz, int maxLength)
{
    char*   pc = sz;
    int     length = 0;
    
    while (*pc && length < maxLength)
    {
        put(writeMask, readMask, true, false, *pc, 43);
        pc++;
        length++;
    }
}


void put_display_address(uint writeMask, uint readMask, uint address)
{
    uint ddAddress = 0b10000000 | address;
    
    put(writeMask, readMask, false, false, ddAddress, 39);
}


void put_lines(uint writeMask, uint readMask, char* szLine1, char* szLine2)
{
    char*   pc;

    put_display_address(writeMask, readMask, 0x00);
    put_string(writeMask, readMask, szLine1, 16);
    put_display_address(writeMask, readMask, 0x40);
    put_string(writeMask, readMask, szLine2, 16);
}

void init_lcd(uint writeMask, uint commandMask)
{
    uint dataMask = make_lcd_mask(false, false, false, 0xff);

    gpio_init_mask(writeMask);
    gpio_set_dir_out_masked(writeMask);

    //Function must be 'put' 3 times to ensure that it is in the correct state.
    put_function(writeMask, commandMask, true, true, false);
    put_function(writeMask, commandMask, true, true, false);
    put_function(writeMask, commandMask, true, true, false);

    put_display(writeMask, commandMask, true, false, false);

    put_shift(writeMask, commandMask, true, false);
}


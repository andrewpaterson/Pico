#include "LCDDisplay.h"
#include "hardware/gpio.h"
#include "GeneralPins.h"
#include "ShiftLCDDisplay.h"


const uint eCycleTime = 10;


void put_lines(S595OutPins* psPins, char* szLine1, char* szLine2);
void put_clear(S595OutPins* psPins);
void init_lcd(S595OutPins* psPins);


uint16_t make_command_value(bool rs, bool rw, bool e, uint data)
{
    uint16_t uiCommand = 0;

    uiCommand |= rs ? 0x01 : 0;
    uiCommand |= rw ? 0x02 : 0;
    uiCommand |= e  ? 0x04 : 0;
    uiCommand |= (data << 3);

    return uiCommand;
}


void wait_for_busy(S595OutPins* psPins)
{
    uint16_t uiBusyLow = make_command_value(false, true, false, 0);
    uint16_t uiBusyHigh = make_command_value(false, true, true, 0);
    shift_out(psPins, uiBusyLow);
    sleep_us_high_power(eCycleTime);

    int count = 6;
    bool busy = true;
    while (busy)
    {
        shift_out(psPins, uiBusyHigh);
        sleep_us_high_power(eCycleTime);
        
        shift_out(psPins, uiBusyLow);
        sleep_us_high_power(eCycleTime);
        count--;
        busy = count != 0;
    }
}


void put(S595OutPins* psPins, bool rs, bool rw, uint data)
{
    uint16_t uiCommandELow = make_command_value(rs, rw, false, data);
    shift_out(psPins, uiCommandELow);
    sleep_us_high_power(eCycleTime);

    uint16_t uiCommandEHigh = make_command_value(rs, rw, true, data);
    shift_out(psPins, uiCommandEHigh);
    sleep_us_high_power(eCycleTime);

    shift_out(psPins, uiCommandELow);    

    wait_for_busy(psPins);
}


void put_function(S595OutPins* psPins, bool eightBit, bool twoLines, bool largeFont)
{
    uint function = 0b00100000;
    uint f2 = eightBit ?  0b00010000 : 0;
    uint f1 = twoLines ?  0b00001000 : 0;
    uint f0 = largeFont ? 0b00000100 : 0;
    
    function = function | f2 | f1 | f0;
    put(psPins, false, false, function);
}


void put_display(S595OutPins* psPins, bool displayOn, bool cursorOn, bool cursoBlink)
{
    uint display =  0b00001000;  
    uint d2 = displayOn ?  0b00000100 : 0;
    uint d1 = cursorOn ?   0b00000010 : 0;
    uint d0 = cursoBlink ? 0b00000001 : 0;
    
    display = display | d2 | d1 | d0;
    put(psPins, false, false, display);
}


void put_shift(S595OutPins* psPins, bool increment, bool shiftDisplay)
{
    uint entry =  0b00000100;
    uint e1 = increment ?   0b00000010 : 0;
    uint e0 = shiftDisplay ? 0b00000001 : 0;
    
    entry = entry | e1 | e0;
    put(psPins, false, false, entry);
}

void put_clear(S595OutPins* psPins)
{
    put(psPins, false, false, 0b00000001);
}


void put_string(S595OutPins* psPins, char* sz, int maxLength)
{
    char*   pc = sz;
    int     length = 0;
    
    while (*pc && length < maxLength)
    {
        put(psPins, true, false, *pc);
        pc++;
        length++;
    }
}


void put_display_address(S595OutPins* psPins, uint address)
{
    uint ddAddress = 0b10000000 | address;
    
    put(psPins, false, false, ddAddress);
}


void put_lines(S595OutPins* psPins, char* szLine1, char* szLine2)
{
    char*   pc;

    put_display_address(psPins, 0x00);
    put_string(psPins, szLine1, 16);
    put_display_address(psPins, 0x40);
    put_string(psPins, szLine2, 16);
}


void init_lcd(S595OutPins* psPins)
{
    //Function must be 'put' 3 times to ensure that it is in the correct state.
    put_function(psPins, true, true, false);
    put_function(psPins, true, true, false);
    put_function(psPins, true, true, false);

    put_display(psPins, true, false, false);

    put_shift(psPins, true, false);

    shift_out(psPins, 0);
}


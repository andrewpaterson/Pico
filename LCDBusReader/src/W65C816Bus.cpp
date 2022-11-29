#include "hardware/gpio.h"
#include "W65C816Bus.h"


void w65_init(SW65C816Pins* psPins)
{
    uint uiPinMask = make_w65c816_pins_mask(psPins, 0xFFFFFFFF, 0xFF);
    gpio_init_mask(uiPinMask);
    gpio_set_dir_in_masked(uiPinMask);
}


void w65_disable_io(SW65C816Pins* psPins)
{
    uint uiPinMask = make_w65c816_pins_mask(psPins, 0xFFFFFFFF, 0xFF);
    gpio_set_dir_in_masked(uiPinMask);
}


void w65_write(SW65C816Pins* psPins, uint uiData, uint uiAddress)
{
    uint uiPinMask = make_w65c816_pins_mask(psPins, 0xFFFFFFFF, 0xFF);
    uint uiValueMask = make_w65c816_pins_mask(psPins, uiAddress, uiData);
    gpio_put_masked(uiPinMask, uiValueMask);
    gpio_set_dir_out_masked(uiPinMask);
}


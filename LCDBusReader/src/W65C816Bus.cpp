#include "hardware/gpio.h"
#include "W65C816Bus.h"


void CW65C816Bus::Init(SW65C816Pins* psPins)
{
    mpsPins = psPins;
    gpio_init_mask(psPins->uiPinMask);
    gpio_set_dir_in_masked(psPins->uiPinMask);
    DisableIO();
}


void CW65C816Bus::DisableIO(void)
{
    gpio_set_dir_in_masked(mpsPins->uiPinMask);
}


void CW65C816Bus::AddressOutDataOut(uint uiData, uint uiAddress)
{
    uint uiValueMask = make_w65c816_pins_mask(mpsPins, uiAddress, uiData);
    gpio_put_masked(mpsPins->uiPinMask, uiValueMask);
    gpio_set_dir_out_masked(mpsPins->uiPinMask);
}


void CW65C816Bus::AddressOutDataIn(uint uiAddress)
{
    gpio_set_dir_in_masked(mpsPins->uiDataMask);

    uint uiValueMask = make_w65c816_pins_mask(mpsPins, uiAddress, 0x00);
    gpio_put_masked(mpsPins->uiAddressMask, uiValueMask);
    gpio_set_dir_out_masked(mpsPins->uiAddressMask);
}


int CW65C816Bus::Read(void)
{
    uint    uiData;
    int     iData;

    uiData = gpio_get_all();

    iData = mask_w65c816_to_data(mpsPins, uiData);
    return iData;
}


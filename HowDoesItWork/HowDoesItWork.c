#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"

const uint PIN_LED_PI = 25;
const uint PIN_CLOCK = 15;

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


int main() 
{
    bi_decl(bi_program_description("How does it work?"));

    stdio_init_all();

    gpio_init(PIN_CLOCK);
    gpio_set_dir(PIN_CLOCK, true);
    gpio_put(PIN_CLOCK, false);

    bool led = true;
    gpio_init(PIN_LED_PI);
    gpio_set_dir(PIN_LED_PI, true);

    while(true)
    {
        gpio_put(PIN_LED_PI, led);
        gpio_put(PIN_CLOCK, led);
        led = !led;
    }
}


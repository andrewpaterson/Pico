#include "GeneralPins.h"


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


                                                                                                                                                                                                                                                                                                                                                                        
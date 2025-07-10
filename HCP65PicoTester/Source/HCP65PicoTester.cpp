#include "TesterPins.h"
#include "TesterState.h"
#include "Commands.h"


#define INPUT_BUFFER_SIZE   256


void BlinkLed(int iMicrosecondDelay)
{
    bool bLed = true;

    for (;;)
    {
        gpio_put(ONBOARD_LED, bLed);
        busy_wait_us_32(iMicrosecondDelay);

        bLed = !bLed;
    }
}





int main(void)
{
    InitPicoPins();

    gpio_put(ENABLE_GPIO, true);
    sleep_us(0);
    gpio_put(ENABLE_GPIO, false);

    gpio_put(ONBOARD_LED, true);

    char szInput[INPUT_BUFFER_SIZE];
    size  uiInputIndex;

    memset(szInput, 0, INPUT_BUFFER_SIZE);
    uiInputIndex = 0;
    gbUseASCIIHex = true;

    stdio_usb_init();
    while (!tusb_inited())
    {
        tight_loop_contents(); // Wait for USB to initialize
    }

    // Main loop
    size x = 0;
    while (true) 
    {
        // Read incoming character from USB serial
        int32 c = getchar_timeout_us(0); // Timeout after 10ms
        if (c != PICO_ERROR_TIMEOUT) 
        {
            if (c == '\n') 
            {
                szInput[uiInputIndex] = '\0';
                ExecuteCommand(szInput, uiInputIndex);
                uiInputIndex = 0;
            }
            else
            {
                szInput[uiInputIndex] = c;
                uiInputIndex++;
            }
        }
        if (x == 1000000)
        {
            printf("Pico USB UART\n");
            x = 0;
        }
        x++;
    }
    
    BlinkLed(100000);
    return 0;
}


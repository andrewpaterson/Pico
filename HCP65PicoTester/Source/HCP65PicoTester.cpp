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


char* ExecuteCommand(const char* szCommand)
{
    size    uiLength;
    char*   szMessage;

    uiLength =  strlen(szCommand);
    szMessage = ExecuteCommand((char*)szCommand, uiLength);

    return szMessage;
}


int main(void)
{
    InitPicoPins();

    gpio_put(ENABLE_GPIO, true);
    sleep_us(0);
    gpio_put(ENABLE_GPIO, false);

    gpio_put(ONBOARD_LED, true);

    char    szInput[INPUT_BUFFER_SIZE];
    size    uiInputIndex;
    char*   szMessage;
    size    uiLength;

    memset(szInput, 0, INPUT_BUFFER_SIZE);
    uiInputIndex = 0;

    ExecuteCommand("P");
    ExecuteCommand("W");
    ExecuteCommand("O");

    stdio_usb_init();
    while (!tusb_inited())
    {
        tight_loop_contents(); // Wait for USB to initialize
    }

    // Main loop
    while (true) 
    {
        // Read incoming character from USB serial
        int32 c = getchar_timeout_us(0); // Timeout after 10ms
        if (c != PICO_ERROR_TIMEOUT) 
        {
            if (c == '\r') 
            {
                if (uiInputIndex > 0)
                {
                    szInput[uiInputIndex] = '\0';
                    szMessage = ExecuteCommand(szInput, uiInputIndex);
                    uiLength = strlen(szMessage);
                    if (uiLength > 0)
                    {
                        szMessage[uiLength] = '\n';
                        szMessage[uiLength+1] = '\0';
                        printf(szMessage);
                    }
                    else
                    {
                        printf("\n");
                    }
                    memset(szInput, 0, INPUT_BUFFER_SIZE);
                    uiInputIndex = 0;
                }
            }
            else if (c == '\0')
            {
                memset(szInput, 0, INPUT_BUFFER_SIZE);
                uiInputIndex = 0;
            }
            else if (c == '\n')
            {
            }
            else
            {
                szInput[uiInputIndex] = c;
                uiInputIndex++;
            }
        }
    }
    
    BlinkLed(100000);
    return 0;
}


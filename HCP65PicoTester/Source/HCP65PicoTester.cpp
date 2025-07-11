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

    ExecuteCommand("W");
    ExecuteCommand("O");
    ExecuteCommand("OFFFFFFFFFFFFFFFF");

    // char    szCommand[100];
    // size    i;
    // uint8   uiRand;

    // for (;;)
    // {
    //     ExecuteCommand("W");
    //     sleep_ms(150);

    //     szCommand[0]='W';
    //     for (i = 0; i < 16; i++)
    //     {
    //         uiRand = (rand() / 10) & 0xf;
    //         szCommand[1 + i] = HexChar(uiRand);
    //     }
    //     szCommand[1 + i]='\0';

    //     ExecuteCommand(szCommand);
    //     sleep_ms(500);
    //     szMessage = ExecuteCommand("R7");
    // }
    
    // BlinkLed(100000);

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
            if (c == '\n' || c == '\r') 
            {
                if (uiInputIndex > 0)
                {
                    szInput[uiInputIndex] = '\0';
                    szMessage = ExecuteCommand(szInput, uiInputIndex);
                    uiLength = strlen(szMessage);
                    if (uiLength > 0)
                    {
                        szMessage[uiLength] = '\r';
                        szMessage[uiLength+1] = '\n';
                        szMessage[uiLength+2] = '\0';
                        printf(szMessage);
                    }
                    memset(szInput, 0, INPUT_BUFFER_SIZE);
                    uiInputIndex = 0;
                }
            }
            else if (c== '\0')
            {
                memset(szInput, 0, INPUT_BUFFER_SIZE);
                uiInputIndex = 0;
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


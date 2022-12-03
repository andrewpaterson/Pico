#ifndef  __KEYPAD__H__
#define  __KEYPAD__H__
#include "ShiftRegister.h"

#define KEYPAD_STAR 0x8000
#define KEYPAD_7    0x4000
#define KEYPAD_4    0x2000
#define KEYPAD_1    0x1000

#define KEYPAD_0    0x0800
#define KEYPAD_8    0x0400
#define KEYPAD_5    0x0200
#define KEYPAD_2    0x0100

#define KEYPAD_HASH 0x0080
#define KEYPAD_9    0x0040
#define KEYPAD_6    0x0020
#define KEYPAD_3    0x0010


class CKeypad
{
protected:
    S165InPins* mpsPins;
    int         miKeyBits;
    int         miPreviousBits;

public:
    void    Init(S165InPins* psPins);
    bool    IsKeyPressed(int iKeyMask);
    bool    IsKeyDown(int iKeyMask);
    void    Read(void);
    char    GetPressed(void);

protected:
    bool    IsPrevDown(int iKeyMask);
};


#endif // __KEYPAD__H__


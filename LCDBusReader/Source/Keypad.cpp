#include  "Keypad.h"


void CKeypad::Init(S165InPins* psPins)
{
    mpsPins = psPins;
    miKeyBits = 0;
    miPreviousBits = 0;
    init_shift(psPins);
}


bool CKeypad::IsKeyDown(int iKeyMask)
{
    if ((miKeyBits & iKeyMask) == iKeyMask)
    {
        return true;
    }
    else
    {
        return false;
    }
}


bool CKeypad::IsPrevDown(int iKeyMask)
{
    if ((miPreviousBits & iKeyMask) == iKeyMask)
    {
        return true;
    }
    else
    {
        return false;
    }
}


bool CKeypad::IsKeyPressed(int iKeyMask)
{
    if (IsKeyDown(iKeyMask) && !IsPrevDown(iKeyMask))
    {
        return true;
    }
    else
    {
        return false;
    }
}


char CKeypad::GetPressed(void)
{
    if (IsKeyPressed(KEYPAD_STAR))
    {
        return '*';
    }
    if (IsKeyPressed(KEYPAD_7))
    {
        return '7';
    }
    if (IsKeyPressed(KEYPAD_4))
    {
        return '4';
    }
    if (IsKeyPressed(KEYPAD_1))
    {
        return '1';
    }
    if (IsKeyPressed(KEYPAD_0))
    {
        return '0';
    }
    if (IsKeyPressed(KEYPAD_8))
    {
        return '8';
    }
    if (IsKeyPressed(KEYPAD_5))
    {
        return '5';
    }
    if (IsKeyPressed(KEYPAD_2))
    {
        return '2';
    }
    if (IsKeyPressed(KEYPAD_HASH))
    {
        return '#';
    }
    if (IsKeyPressed(KEYPAD_9))
    {
        return '9';
    }
    if (IsKeyPressed(KEYPAD_6))
    {
        return '6';
    }
    if (IsKeyPressed(KEYPAD_3))
    {
        return '3';
    }

    return '\0';
}


void CKeypad::Read(void)
{
    miPreviousBits = miKeyBits;
    miKeyBits = shift_in(mpsPins);
}

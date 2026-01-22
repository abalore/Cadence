#include "Headers/Flag.h"

Flag::Flag(BYTE *s, BYTE m)
{
    storage = s;
    mask = m;
    nmask = (BYTE)(m ^ 0xFF);
}

void Flag::Set(bool value)
{
    if (value)
        *storage |= mask;
    else
        *storage &= nmask;
}

bool Flag::Get()
{
    return (*storage & mask) > 0;
}

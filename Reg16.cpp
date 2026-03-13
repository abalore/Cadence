#include "Reg16.h"

Reg16::Reg16(BYTE *h, BYTE *l)
{
    H = h;
    L = l;
}

word Reg16::Get()
{
    return *H * 256 + *L;
}

void Reg16::Set(word value)
{
    *H = (value / 256);
    *L = (value & 0xFF);
}

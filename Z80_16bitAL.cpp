#include "Z80.h"

bool Z80::ADD_HL_vv(word w)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::ALU;
        w1 = HL.Get();
        fC = w1 + w > 0xFFFF;
        fH = (w1 & 0xFFF) + (w & 0xFFF) > 0xFFF;
        break;
    case 2:
        HL.Set(w1 + w);
        break;
    case 3:
        fN = false;
        f3 = H & 0x08;
        f5 = H & 0x20;
        return true;
    }
    return false;
}

bool Z80::ADC_HL_vv(word w)
{
    switch(mCycle)
    {
    case 1:
        w1 = HL.Get();
        w2 = w + (fC ? 1 : 0);
        w3 = w1 + w2;
        mCycleType = MCycleType::ALU;
        fC = w1 + w2 > 0xFFFF;
        fP = ((w1 ^ w3) & (w2 ^ w3)) & 0x8000;
        fH = (w1 & 0xFFF) + (w2 & 0xFFF) > 0xFFF;
        break;
    case 2:
        HL.Set(w3);
        break;
    case 3:
        fN = false;
        fS = H & 0x80;
        fZ = w3 == 0;
        f3 = H & 0x08;
        f5 = H & 0x20;
        return true;
    }
    return false;
}

bool Z80::SBC_HL_vv(word w)
{
    switch(mCycle)
    {
    case 1:
        w1 = HL.Get();
        w2 = w + (fC ? 1 : 0);
        w3 = w1 - w2;
        mCycleType = MCycleType::ALU;
        fC = w2 > w1;
        fP = ((w1 ^ w2) & (w1 ^ w3)) & 0x8000;
        fH = ((w1 & 0xFFF) - (w2 & 0xFFF)) & 0x1000;
        break;
    case 2:
        HL.Set(w3);
        break;
    case 3:
        fN = true;
        fZ = w3 == 0;
        fS = H & 0x80;
        f3 = H & 0x08;
        f5 = H & 0x20;
        return true;
    }
    return false;
}

bool Z80::INC_RR(Reg16 reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::END16ALU;
        *reg.L = *reg.L + 1;
        if (*reg.L == 0)
            *reg.H = *reg.H + 1;
        return true;
    }
    return false;
}

bool Z80::INC_WW(word &w)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::END16ALU;
        w++;
        return true;
    }
    return false;
}

bool Z80::DEC_RR(Reg16 reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::END16ALU;
        *reg.L = *reg.L - 1;
        if (*reg.L == 0xff)
            *reg.H = *reg.H - 1;
        return true;
    }
    return false;
}

bool Z80::DEC_WW(word &w)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::END16ALU;
        w--;
        return true;
    }
    return false;

}

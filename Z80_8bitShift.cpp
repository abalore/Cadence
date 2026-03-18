#include "Z80.h"

void Z80::SetFlagsAfterShiftOp(BYTE b)
{
    fP = GetParity(b);
    fS = (b & 0x80) > 0;
    fZ = b == 0;
    fH = false;
    fN = false;
    f3 = b & 0x08;
    f5 = b & 0x20;
}

void Z80::RLC_R(BYTE &reg)
{
    fC = (reg & 0x80) > 0;
    reg <<= 1;
    if (fC) reg++;
    SetFlagsAfterShiftOp(reg);
}

void Z80::RRC_R(BYTE &reg)
{
    fC = (reg & 0x01) > 0;
    reg >>= 1;
    if (fC) reg += 0x80;
    SetFlagsAfterShiftOp(reg);
}

void Z80::RL_R(BYTE &reg)
{
    tC = fC;
    fC = (reg & 0x80) > 0;
    reg <<= 1;
    if (tC) reg++;
    SetFlagsAfterShiftOp(reg);
}

void Z80::RR_R(BYTE &reg)
{
    tC = fC;
    fC = (reg & 0x01) > 0;
    reg >>= 1;
    if (tC) reg += 0x80;
    SetFlagsAfterShiftOp(reg);
}

void Z80::SLA_R(BYTE &reg)
{
    fC = (reg & 0x80) > 0;
    reg <<= 1;
    SetFlagsAfterShiftOp(reg);
}

void Z80::SRA_R(BYTE &reg)
{
    tC = (reg & 0x80) > 0;
    fC = (reg & 0x01) > 0;
    reg >>= 1;
    if (tC) reg += 0x80;
    SetFlagsAfterShiftOp(reg);
}

void Z80::SLL_R(BYTE &reg)
{
    fC = (reg & 0x80) > 0;
    reg <<= 1;
    reg += 1;
    SetFlagsAfterShiftOp(reg);
}

void Z80::SRL_R(BYTE &reg)
{
    fC = (reg & 0x01) > 0;
    reg >>= 1;
    SetFlagsAfterShiftOp(reg);
}

bool Z80::ShiftOpIndHL(BYTE opCode)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = HL.Get();
        break;
    case 2:
        mCycleType = MCycleType::WRITE;
        switch(opCode)
        {
        case 0x06: RLC_R(DR); break;
        case 0x0E: RRC_R(DR); break;
        case 0x16: RL_R(DR); break;
        case 0x1E: RR_R(DR); break;
        case 0x26: SLA_R(DR); break;
        case 0x2E: SRA_R(DR); break;
        case 0x36: SLL_R(DR); break;
        case 0x3E: SRL_R(DR); break;
        }
        break;
    case 3:
        return true;
    }
    return false;
}

bool Z80::SET_x_Ind_HL(int X)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = HL.Get();
        break;
    case 2:
        mCycleType = MCycleType::WRITE;
        SET_x_R(X, &DR);
        break;
    case 3:
        return true;
    }
    return false;
}

bool Z80::RES_x_Ind_HL(int X)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = HL.Get();
        break;
    case 2:
        mCycleType = MCycleType::WRITE;
        RES_x_R(X, &DR);
        break;
    case 3:
        return true;
    }
    return false;
}

bool Z80::BIT_x_Ind_HL(int X)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = HL.Get();
        break;
    case 2:
        mCycleType = MCycleType::WRITE;
        BIT_x_R(X, &DR);
        f3 = H & 0x08;
        f5 = H & 0x20;
        return true;
    }
    return false;
}


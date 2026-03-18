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

bool Z80::ShiftBitOpIndHL(BYTE opCode)
{
    switch(mCycle)
    {
    case 2:
        mCycleType = MCycleType::READ;
        AR = HL.Get();
        break;
    case 3:
        mCycleType = MCycleType::WRITE;
        intAlign = true;
        switch(opCode)
        {
        case 0x06:
            RLC_R(DR);
            break;
        case 0x0E:
            RRC_R(DR);
            break;
        case 0x16:
            RL_R(DR);
            break;
        case 0x1E:
            RR_R(DR);
            break;
        case 0x26:
            SLA_R(DR);
            break;
        case 0x2E:
            SRA_R(DR);
            break;
        case 0x36:
            SLL_R(DR);
            break;
        case 0x3E:
            SRL_R(DR);
            break;
        case 0x46:
            BIT_x_R(0, &DR);
            f3 = H & 0x08;
            f5 = H & 0x20;
            return true;
        case 0x4E:
            BIT_x_R(1, &DR);
            f3 = H & 0x08;
            f5 = H & 0x20;
            return true;
        case 0x56:
            BIT_x_R(2, &DR);
            f3 = H & 0x08;
            f5 = H & 0x20;
            return true;
        case 0x5E:
            BIT_x_R(3, &DR);
            f3 = H & 0x08;
            f5 = H & 0x20;
            return true;
        case 0x66:
            BIT_x_R(4, &DR);
            f3 = H & 0x08;
            f5 = H & 0x20;
            return true;
        case 0x6E:
            BIT_x_R(5, &DR);
            f3 = H & 0x08;
            f5 = H & 0x20;
            return true;
        case 0x76:
            BIT_x_R(6, &DR);
            f3 = H & 0x08;
            f5 = H & 0x20;
            return true;
        case 0x7E:
            BIT_x_R(7, &DR);
            f3 = H & 0x08;
            f5 = H & 0x20;
            return true;
        case 0x86:
            RES_x_R(0, &DR);
            break;
        case 0x8E:
            RES_x_R(1, &DR);
            break;
        case 0x96:
            RES_x_R(2, &DR);
            break;
        case 0x9E:
            RES_x_R(3, &DR);
            break;
        case 0xA6:
            RES_x_R(4, &DR);
            break;
        case 0xAE:
            RES_x_R(5, &DR);
            break;
        case 0xB6:
            RES_x_R(6, &DR);
            break;
        case 0xBE:
            RES_x_R(7, &DR);
            break;
        case 0xC6:
            SET_x_R(0, &DR);
            break;
        case 0xCE:
            SET_x_R(1, &DR);
            break;
        case 0xD6:
            SET_x_R(2, &DR);
            break;
        case 0xDE:
            SET_x_R(3, &DR);
            break;
        case 0xE6:
            SET_x_R(4, &DR);
            break;
        case 0xEE:
            SET_x_R(5, &DR);
            break;
        case 0xF6:
            SET_x_R(6, &DR);
            break;
        case 0xFE:
            SET_x_R(7, &DR);
            break;
        }
        break;
    case 4:
        return true;
    }
    return false;
}

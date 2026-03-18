#include "Z80.h"

bool Z80::LD_Ind_RR(Reg16 reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = PC++;
        break;
    case 2:
        t8 = DR;
        AR = PC++;
        break;
    case 3:
        AR = DR * 256 + t8;
        mCycleType = MCycleType::WRITE;
        DR = *reg.L;
        break;
    case 4:
        AR++;
        DR = *reg.H;
        break;
    case 5:
        return true;
    }
    return false;
}

bool Z80::LD_Ind_WW(word w)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = PC++;
        break;
    case 2:
        t8 = DR;
        AR = PC++;
        break;
    case 3:
        AR = DR * 256 + t8;
        mCycleType = MCycleType::WRITE;
        DR = w & 0x00FF;
        break;
    case 4:
        AR++;
        DR = w / 0x100;
        break;
    case 5:
        return true;
    }
    return false;
}

bool Z80::LD_RR_Ind(Reg16 reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = PC++;
        break;
    case 2:
        t8 = DR;
        AR = PC++;
        break;
    case 3:
        AR = DR * 256 + t8;
        break;
    case 4:
        *reg.L = DR;
        AR++;
        break;
    case 5:
        *reg.H = DR;
        return true;
    }
    return false;
}

bool Z80::LD_WW_Ind(word &w)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = PC++;
        break;
    case 2:
        t8 = DR;
        AR = PC++;
        break;
    case 3:
        AR = DR * 256 + t8;
        break;
    case 4:
        w = DR;
        AR++;
        break;
    case 5:
        w += DR * 256;
        return true;
    }
    return false;
}


bool Z80::LD_Ind_A()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = PC++;
        break;
    case 2:
        t8 = DR;
        AR = PC++;
        break;
    case 3:
        AR = DR * 256 + t8;
        mCycleType = MCycleType::WRITE;
        DR = A;
        break;
    case 4:
        return true;
    }
    return false;
}

bool Z80::LD_A_Ind()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = PC++;
        break;
    case 2:
        t8 = DR;
        AR = PC++;
        break;
    case 3:
        AR = DR * 256 + t8;
        break;
    case 4:
        A = DR;
        return true;
    }
    return false;
}

bool Z80::LD_R_Ind_HL(BYTE &reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = HL.Get();
        break;
    case 2:
        reg = DR;
        return true;
    }
    return false;
}

bool Z80::LD_Ind_HL_R(BYTE &reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::WRITE;
        AR = HL.Get();
        DR = reg;
        break;
    case 2:
        return true;
    }
    return false;
}

bool Z80::LD_RR_nn(Reg16 reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = PC++;
        break;
    case 2:
        *reg.L = DR;
        AR = PC++;
        break;
    case 3:
        *reg.H = DR;
        return true;
    }
    return false;
}

bool Z80::LD_WW_nn(word &w)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = PC++;
        break;
    case 2:
        w = DR;
        AR = PC++;
        break;
    case 3:
        w += DR * 256;
        return true;
    }
    return false;
}



bool Z80::LD_R_n(BYTE &reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = PC++;
        break;
    case 2:
        reg = DR;
        return true;
    }
    return false;
}

bool Z80::LD_Ind_RR_A(Reg16 reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::WRITE;
        AR = reg.Get();
        DR = A;
        break;
    case 2:
        return true;
    }
    return false;
}

bool Z80::PUSH_RR(Reg16 reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::ALU1;
        break;
    case 2:
        mCycleType = MCycleType::WRITE;
        AR = --SP;
        DR = *reg.H;
        break;
    case 3:
        AR = --SP;
        DR = *reg.L;
        break;
    case 4:
        return true;
    }
    return false;
}

bool Z80::POP_RR(Reg16 reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = SP;
        break;
    case 2:
        *reg.L = DR;
        AR++;
        break;
    case 3:
        *reg.H = DR;
        AR++;
        SP = AR;
        return true;
    }
    return false;
}

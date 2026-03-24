#include "Z80.h"

#define FinishInstruction { mCycleType = MCycleType::FETCH; idMode = IDMode::BASIC; }

bool Z80::DJNZ()
{
    switch(mCycle)
    {
    case 1:
        AR = PC++;
        mCycleType = MCycleType::READ;
        break;
    case 2:
        B--;
        if (B == 0)
            return true;
        else
            PC += (sbyte) DR;
        mCycleType = MCycleType::ALU4;
        break;
    case 3:
        return true;
    }
    return false;
}

bool Z80::JR(bool condition)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = PC++;
        break;
    case 2:
        if (!condition)
            return true;
        else
            mCycleType = MCycleType::RELADDR;
        AR = PC;
        break;
    case 3:
        PC = AR;
        return true;
    }
    return false;
}

bool Z80::RET()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = SP;
        break;
    case 2:
        w1 = DR;
        AR++;
        break;
    case 3:
        w1 += DR * 256;
        AR++;
        SP = AR;
        PC = w1;
        return true;
    }
    return false;
}

bool Z80::RET(bool condition)
{
    switch(mCycle)
    {
    case 1:
        if (!condition)
            return true;
        mCycleType = MCycleType::READ;
        AR = SP++;
        break;
    case 2:
        w1 = DR;
        AR = SP++;
        break;
    case 3:
        w1 += DR * 256;
        PC = w1;
        return true;
    }
    return false;
}

bool Z80::JP(bool condition)
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
        if (condition)
            PC = DR * 256 + t8;
        return true;
    }
    return false;
}

bool Z80::CALL(bool condition)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = PC++;
        break;
    case 2:
        *t16.L = DR;
        AR = PC++;
        break;
    case 3:
        *t16.H = DR;
        if (condition)
        {
            mCycleType = MCycleType::WRITE;
            AR = SP;
            AR--;
            DR = (BYTE)(PC >> 8);
        }
        else
            return true;
        break;
    case 4:
        AR--;
        DR = (BYTE)(PC & 0xFF);
        break;
    case 5:
        SP = AR;
        PC = t16.Get();
        return true;
    }
    return false;
}

bool Z80::RST(BYTE address)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::WRITE;
        AR = --SP;
        DR = (BYTE)(PC >> 8);
        break;
    case 2:
        AR = --SP;
        DR = (BYTE)(PC & 0xFF);
        break;
    case 3:
        PC = address;
        return true;
    }
    return false;
}


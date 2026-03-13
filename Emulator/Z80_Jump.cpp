#include "Headers/Z80.h"

#define FinishInstruction { mCycleType = MCycleType::FETCH; idMode = IDMode::BASIC; }

void Z80::JR(bool condition)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = PC++;
        break;
    case 2:
        if (!condition)
        {
            FinishInstruction
        }
        else
            mCycleType = MCycleType::ALU;
        break;
    case 3:
        intAlign = true;
        PC += (sbyte)DR;
        FinishInstruction
            break;
    }
}

void Z80::RET()
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
        intAlign = true;
        w1 += DR * 256;
        AR++;
        SP = AR;
        PC = w1;
        FinishInstruction
            break;
    }
}

void Z80::RET(bool condition)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = SP;
        break;
    case 2:
        if (!condition)
            FinishInstruction
                w1 = DR;
        AR++;
        break;
    case 3:
        w1 += DR * 256;
        AR++;
        mCycleType = MCycleType::ALU;
        break;
    case 4:
        intAlign = true;
        SP = AR;
        PC = w1;
        FinishInstruction
            break;
    }
}

void Z80::JP(bool condition)
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
        intAlign = true;
        AR++;
        if (condition)
            PC = DR * 256 + t8;
        FinishInstruction
            break;
    }
}

void Z80::CALL(bool condition)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = PC;
        break;
    case 2:
        *t16.L = DR;
        AR++;
        break;
    case 3:
        intAlign = true;
        *t16.H = DR;
        AR++;
        PC = AR;
        if (condition)
        {
            mCycleType = MCycleType::WRITE;
            AR = SP;
            AR--;
            DR = (BYTE)(PC >> 8);
        }
        else
            FinishInstruction
                break;
    case 4:
        AR--;
        DR = (BYTE)(PC & 0xFF);
        break;
    case 5:
        SP = AR;
        PC = t16.Get();
        FinishInstruction
            break;
    }
}

void Z80::RST(BYTE address)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::WRITE;
        AR = SP;
        AR--;
        DR = (BYTE)(PC >> 8);
        break;
    case 2:
        AR--;
        DR = (BYTE)(PC & 0xFF);
        break;
    case 3:
        mCycleType = MCycleType::ALU;
        SP = AR;
        break;
    case 4:
        PC = address;
        intAlign = true;
        break;
    case 5:
        FinishInstruction
            break;
    }
}


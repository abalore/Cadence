#include "Z80.h"
#include "GateArray.h"

bool Z80::Step_Int_Exec()
{
    switch(im)
    {
    case 0: ///////////////////////// Implement real mode 0
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
            SP = AR;
            DR = (BYTE)(PC & 0xFF);
            break;
        case 3:
            GateArray::AckInt();
            PC = 0x0038;
            mCycleType = MCycleType::ALU;
            break;
        case 4:
            mCycleType = MCycleType::FETCH;
            idMode = IDMode::BASIC;
            break;
        }
        break;
    case 1:
        switch(mCycle)
        {
        case 1:
            GateArray::AckInt();
            mCycleType = MCycleType::ALU;
            break;
        case 2:
            mCycleType = MCycleType::WRITE;
            AR = SP;
            AR--;
            DR = (BYTE)(PC >> 8);
            break;
        case 3:
            AR--;
            SP = AR;
            DR = (BYTE)(PC & 0xFF);
            break;
        case 4:
            mCycleType = MCycleType::ALU;
            PC = 0x0038;
            if (!intAlign)
            {
                mCycleType = MCycleType::FETCH;
                idMode = IDMode::BASIC;
            }
            break;
        case 5:
            mCycleType = MCycleType::FETCH;
            idMode = IDMode::BASIC;
            break;
        }
        break;
    case 2:
        switch(mCycle)
        {
        case 1:
            GateArray::AckInt();
            mCycleType = MCycleType::WRITE;
            AR = SP;
            AR--;
            DR = (BYTE)(PC >> 8);
            break;
        case 2:
            AR--;
            SP = AR;
            DR = (BYTE)(PC & 0xFF);
            break;
        case 3:
            AR = I * 256;
            mCycleType = MCycleType::READ;
            break;
        case 4:
            *t16.L = DR;
            AR++;
            break;
        case 5:
            *t16.H = DR;
            PC = t16.Get();
            mCycleType = MCycleType::FETCH;
            idMode = IDMode::BASIC;
            break;
        }
        break;
    }
    return false;
}

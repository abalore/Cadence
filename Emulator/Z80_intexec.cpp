#include "Headers/Z80.h"
#include "Headers/GateArray.h"

void Z80::Step_Int_Exec()
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

}

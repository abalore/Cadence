#include "Z80.h"

void Z80::Step_IDX_3()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = PC++;
        break;
    case 2:
        index = (sbyte)DR;
        AR = PC++;
        break;
    case 3:
        AR = IDX->Get() + index;
        mCycleType = MCycleType::WRITE;
        break;
    case 4:
        mCycleType = MCycleType::ALU;
        break;
    case 5:
        mCycleType = MCycleType::FETCH;
        idMode = IDMode::BASIC;
        intAlign = true;
        break;
    }
}

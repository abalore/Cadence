#include "Z80.h"

bool Z80::Step_IDX_3()
{
    switch(mCycle)
    {
    case 1: // 4T
        mCycleType = MCycleType::READ;
        AR = PC++;
        break;
    case 2: // 3T
        index = (sbyte)DR;
        AR = PC++;
        mCycleType = MCycleType::READ5;
        break;
    case 3: // 5T
        AR = IDX->Get() + index;
        mCycleType = MCycleType::WRITE;
        break;
    case 4: // 3T
        return true;
    }
    return false;
}

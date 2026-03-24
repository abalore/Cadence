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
        AR = IDX->Get();
        mCycleType = MCycleType::RELADDR;
        break;
    case 3: // 5T
        w1 = AR;
        AR = PC++;
        mCycleType = MCycleType::READ;
        break;
    case 4: // 3T
        AR = w1;
        mCycleType = MCycleType::WRITE;
        break;
    case 5: // 3T
        return true;
    }
    return false;
}

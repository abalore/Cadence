#include "Z80.h"

bool Z80::Step_IDX_3()
{
    switch(mCycle)
    {
    case 1: // Fetched the op (4T)
        mCycleType = MCycleType::READ;
        AR = PC++;
        break;
    case 2: // Loaded index (3T)
        index = (sbyte)DR;
        AR = PC++;
        break;
    case 3: // Extend 2T
        AR = IDX->Get() + index;
        mCycleType = MCycleType::ALU2;
        break;
    case 4: // Computed address and loaded value (Total 5T)
        mCycleType = MCycleType::WRITE;
        break;
    case 5: // Wrote n (4T)
        return true;
    }
    return false;
}

#include "Z80.h"

bool Z80::Step_IDX_2()
{
    switch(mCycle)
    {
    case 1: // M2 4T
        AR = PC++;
        mCycleType = MCycleType::READ;
        break;
    case 2: // M3 3T
        AR = IDX->Get();
        mCycleType = MCycleType::RELADDR;
        break;
    case 3: // M4 5T
        if (IR >= 0x70 && IR <= 0x77)
        {
            switch(IR & 0x0F)
            {
            case 0x0: DR = B; break;
            case 0x1: DR = C; break;
            case 0x2: DR = D; break;
            case 0x3: DR = E; break;
            case 0x4: DR = H; break;
            case 0x5: DR = L; break;
            case 0x7: DR = A; break;
            }
            mCycleType = MCycleType::WRITE;
        }
        else
            mCycleType = MCycleType::READ;
        break;
    case 4: // M5 3T
        switch(IR >> 4)
        {
        case 0x4:
            switch(IR & 0x0F)
            {
            case 0x6: B = DR; break;
            case 0xE: C = DR; break;
            }
            break;
        case 0x5:
            switch(IR & 0x0F)
            {
            case 0x6: D = DR; break;
            case 0xE: E = DR; break;
            }
            break;
        case 0x6:
            switch(IR & 0x0F)
            {
            case 0x6: H = DR; break;
            case 0xE: L = DR; break;
            }
            break;
        case 0x7:
            switch(IR & 0x0F)
            {
            case 0xE: A = DR; break;
            }
        }
        return true;
    }
    return false;
}

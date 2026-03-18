#include "Z80.h"

bool Z80::Step_Int_Exec()
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
        SP = AR;
        DR = (BYTE)(PC & 0xFF);
        break;
    case 3:
        switch(im)
        {
        case 0:
            PC = t8 & 0b00111000;
            return true;
        case 1:
            PC = 0x0038;
            return true;
        case 2:
            AR = I * 256 + t8;
            mCycleType = MCycleType::READ;
            break;
        }
        break;
    case 4:
        *t16.L = DR;
        AR++;
        break;
    case 5:
        *t16.H = DR;
        PC = t16.Get();
        return true;
    }
    return false;
}

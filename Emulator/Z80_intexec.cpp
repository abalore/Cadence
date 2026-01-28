#include "Headers/Z80.h"

void Z80::Step_Int_Exec()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::WRITE;
        tAddr = SP.Get();
        tAddr--;
        DR = (BYTE)(PC >> 8);
        break;
    case 2:
        tAddr--;
        SP.Set(tAddr);
        DR = (BYTE)(PC & 0xFF);
        break;
    case 3:
        switch(InterruptMode)
        {
        case 0:
        case 1:
            PC = 0x0038;
            mCycleType = MCycleType::ALU;
            break;
        case 2:
            tAddr = I * 256;
            mCycleType = MCycleType::READ;
            break;
        }
        break;
    case 4:
        switch(InterruptMode)
        {
        case 2:
            *t16.L = DR;
            tAddr++;
            break;
        }
        break;
    case 5:
        switch(InterruptMode)
        {
        case 2:
            *t16.H = DR;
            PC = t16.Get();
            break;
        }
        FinishInstruction();
        break;
    }
}

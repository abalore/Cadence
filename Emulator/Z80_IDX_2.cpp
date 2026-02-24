#include "Headers/Z80.h"

#define FinishInstruction { mCycleType = MCycleType::FETCH; idMode = IDMode::BASIC; }

void Z80::Step_IDX_2()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = PC++;
        break;
    case 2:
        AR = IDX->Get();
        AR += (sbyte)DR;
        break;
    case 3:
        mCycleType = MCycleType::ALU;
        switch(IR >> 4)
        {
        case 0x4:
            switch(IR & 0x0F)
            {
            case 0x6: // 5
                B = DR;
                break;
            case 0xE: // 5
                C = DR;
                break;
            }
            break;
        case 0x5:
            switch(IR & 0x0F)
            {
            case 0x6: // 5
                D = DR;
                break;
            case 0xE: // 5
                E = DR;
                break;
            }
            break;
        case 0x6:
            switch(IR & 0x0F)
            {
            case 0x6: // 5
                H = DR;
                break;
            case 0xE: // 5
                L = DR;
                break;
            }
            break;
        case 0x7:
            switch(IR & 0x0F)
            {
            case 0x0: // LD (IDX+d),B
                mCycleType = MCycleType::WRITE;
                DR = B;
                break;
            case 0x1: // LD (IDX+d),C
                mCycleType = MCycleType::WRITE;
                DR = C;
                break;
            case 0x2: // LD (IDX+d),D
                mCycleType = MCycleType::WRITE;
                DR = D;
                break;
            case 0x3: // LD (IDX+d),E
                mCycleType = MCycleType::WRITE;
                DR = E;
                break;
            case 0x4: // LD (IDX+d),H
                mCycleType = MCycleType::WRITE;
                DR = H;
                break;
            case 0x5: // LD (IDX+d),L
                mCycleType = MCycleType::WRITE;
                DR = L;
                break;
            case 0x7: // LD (IDX+d),A
                mCycleType = MCycleType::WRITE;
                DR = A;
                break;
            case 0xE: // LD A,(IDX+d)
                A = DR; // 5
                break;
            }
            break;
        }
        break;
    case 4:
            FinishInstruction
            break;
    }
}

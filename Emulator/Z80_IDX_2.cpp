#include "Headers/Z80.h"

void Z80::Step_IDX_2()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = PC;
        break;
    case 2:
        index = DR;
        tAddr++;
        PC = tAddr;
        tAddr = IDX->Get() + index;
        break;
    case 3:
        t8 = DR;
        switch(IR >> 4)
        {
        case 0x3:
            switch(IR & 0x0F)
            {
            case 0x4: // 6
                INC_R(t8);
                break;
            case 0x5: // 6
                DEC_R(t8);
                break;
            }
            break;
        case 0x4:
            switch(IR & 0x0F)
            {
            case 0x6: // 5
                B = t8;
                break;
            case 0xE: // 5
                C = t8;
                break;
            }
            break;
        case 0x5:
            switch(IR & 0x0F)
            {
            case 0x6: // 5
                D = t8;
                break;
            case 0xE: // 5
                E = t8;
                break;
            }
            break;
        case 0x6:
            switch(IR & 0x0F)
            {
            case 0x6: // 5
                H = t8;
                break;
            case 0xE: // 5
                L = t8;
                break;
            }
            break;
        case 0x7:
            switch(IR & 0x0F)
            {
            case 0x0: // LD (IDX+d),B
                t8 = B; // 5
                break;
            case 0x1: // LD (IDX+d),C
                t8 = C; // 5
                break;
            case 0x2: // LD (IDX+d),D
                t8 = D; // 5
                break;
            case 0x3: // LD (IDX+d),E
                t8 = E; // 5
                break;
            case 0x4: // LD (IDX+d),H
                t8 = H; // 5
                break;
            case 0x5: // LD (IDX+d),L
                t8 = L; // 5
                break;
            case 0x7: // LD (IDX+d),A
                t8 = A; // 5
                break;
            case 0xE: // LD A,(IX+d)
                A = t8; // 5
                break;
            }
            break;
        case 0x8:
            switch(IR & 0x0F)
            {
            case 0x6:
                ADD_R(t8); // 5
                break;
            case 0xE:
                ADC_R(t8); // 5
                break;
            }
            break;
        case 0x9:
            switch(IR & 0x0F)
            {
            case 0x6:
                SUB_R(t8); // 5
                break;
            case 0xE:
                SBC_R(t8); // 5
                break;
            }
            break;
        case 0xA:
            switch(IR & 0x0F)
            {
            case 0x6:
                AND_R(t8); // 5
                break;
            case 0xE:
                XOR_R(t8); // 5
                break;
            }
            break;
        case 0xB:
            switch(IR & 0x0F)
            {
            case 0x6:
                OR_R(t8); // 5
                break;
            case 0xE:
                CP_R(t8); // 5
                break;
            }
            break;
        }
        mCycleType = MCycleType::WRITE;
        DR = t8;
        break;
    case 4:
        if ((IR >> 4) == 3)
            FinishInstruction();
        else
            mCycleType = MCycleType::ALU;
        break;
    case 5:
        FinishInstruction();
        break;
    }
}

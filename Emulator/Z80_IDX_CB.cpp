#include "Headers/Z80.h"

void Z80::Step_IDX_CB()
{
    switch(mCycle)
    {
    case 1:
        //PC--;
        mCycleType = MCycleType::READ;
        AR = PC;
        break;
    case 2:
        index = (sbyte)DR;
        AR++;
        break;
    case 3:
        t8 = DR;
        AR++;
        PC = AR;
        AR = IDX->Get() + index;
        intAlign = true;
        break;
    case 4:
        mCycleType = MCycleType::WRITE;
        tByte = DR;
        switch(t8 >> 4)
        {
        case 0x0:
            switch(t8 & 0x0F)
            {
            case 0x0: // RLC (IX+d),B
                RLC_IDX_R(B);
                break;
            case 0x1: // RLC (IX+d),C
                RLC_IDX_R(C);
                break;
            case 0x2: // RLC (IX+d),D
                RLC_IDX_R(D);
                break;
            case 0x3: // RLC (IX+d),E
                RLC_IDX_R(E);
                break;
            case 0x4: // RLC (IX+d),H
                RLC_IDX_R(H);
                break;
            case 0x5: // RLC (IX+d),L
                RLC_IDX_R(L);
                break;
            case 0x6: // RLC (IX+d)
                RLC_IDX_R(t8);
                break;
            case 0x7: // RLC (IX+d),A
                RLC_IDX_R(A);
                break;
            case 0x8: // RRC (IX+d),B
                RRC_IDX_R(B);
                break;
            case 0x9: // RRC (IX+d),C
                RRC_IDX_R(C);
                break;
            case 0xA: // RRC (IX+d),D
                RRC_IDX_R(D);
                break;
            case 0xB: // RRC (IX+d),E
                RRC_IDX_R(E);
                break;
            case 0xC: // RRC (IX+d),H
                RRC_IDX_R(H);
                break;
            case 0xD: // RRC (IX+d),L
                RRC_IDX_R(L);
                break;
            case 0xE: // RRC (IX+d)
                RRC_IDX_R(t8);
                break;
            case 0xF: // RRC (IX+d),A
                RRC_IDX_R(A);
                break;
            }
            break;
        case 0x1:
            switch(t8 & 0x0F)
            {
            case 0x0: // RL B
                RL_IDX_R(B);
                break;
            case 0x1: // RL C
                RL_IDX_R(C);
                break;
            case 0x2: // RL D
                RL_IDX_R(D);
                break;
            case 0x3: // RL E
                RL_IDX_R(E);
                break;
            case 0x4: // RL H
                RL_IDX_R(H);
                break;
            case 0x5: // RL L
                RL_IDX_R(L);
                break;
            case 0x6: // RL (HL)
                RL_IDX_R(t8);
                break;
            case 0x7: // RL A
                RL_IDX_R(A);
                break;
            case 0x8: // RR B
                RR_IDX_R(B);
                break;
            case 0x9: // RR C
                RR_IDX_R(C);
                break;
            case 0xA: // RR D
                RR_IDX_R(D);
                break;
            case 0xB: // RR E
                RR_IDX_R(E);
                break;
            case 0xC: // RR H
                RR_IDX_R(H);
                break;
            case 0xD: // RR L
                RR_IDX_R(L);
                break;
            case 0xE: // RR (HL)
                RR_IDX_R(t8);
                break;
            case 0xF: // RR A
                RR_IDX_R(A);
                break;
            }
            break;
        case 0x2:
            switch(t8 & 0x0F)
            {
            case 0x0: // SLA B
                SLA_IDX_R(B);
                break;
            case 0x1: // SLA C
                SLA_IDX_R(C);
                break;
            case 0x2: // SLA D
                SLA_IDX_R(D);
                break;
            case 0x3: // SLA E
                SLA_IDX_R(E);
                break;
            case 0x4: // SLA H
                SLA_IDX_R(H);
                break;
            case 0x5: // SLA L
                SLA_IDX_R(L);
                break;
            case 0x6: // SLA (HL)
                SLA_IDX_R(t8);
                break;
            case 0x7: // SLA A
                SLA_IDX_R(A);
                break;
            case 0x8: // SRA B
                SRA_IDX_R(B);
                break;
            case 0x9: // SRA C
                SRA_IDX_R(C);
                break;
            case 0xA: // SRA D
                SRA_IDX_R(D);
                break;
            case 0xB: // SRA E
                SRA_IDX_R(E);
                break;
            case 0xC: // SRA H
                SRA_IDX_R(H);
                break;
            case 0xD: // SRA L
                SRA_IDX_R(L);
                break;
            case 0xE: // SRA (HL)
                SRA_IDX_R(t8);
                break;
            case 0xF: // SRA A
                SRA_IDX_R(A);
                break;
            }
            break;
        case 0x3:
            switch(t8 & 0x0F)
            {
            case 0x0: // SLL B
                SLL_IDX_R(B);
                break;
            case 0x1: // SLL C
                SLL_IDX_R(C);
                break;
            case 0x2: // SLL D
                SLL_IDX_R(D);
                break;
            case 0x3: // SLL E
                SLL_IDX_R(E);
                break;
            case 0x4: // SLL H
                SLL_IDX_R(H);
                break;
            case 0x5: // SLL L
                SLL_IDX_R(L);
                break;
            case 0x6: // SLL (HL)
                SLL_IDX_R(t8);
                break;
            case 0x7: // SLL A
                SLL_IDX_R(A);
                break;
            case 0x8: // SRL B
                SRL_IDX_R(B);
                break;
            case 0x9: // SRL C
                SRL_IDX_R(C);
                break;
            case 0xA: // SRL D
                SRL_IDX_R(D);
                break;
            case 0xB: // SRL E
                SRL_IDX_R(E);
                break;
            case 0xC: // SRL H
                SRL_IDX_R(H);
                break;
            case 0xD: // SRL L
                SRL_IDX_R(L);
                break;
            case 0xE: // SRL (HL)
                SRL_IDX_R(t8);
                break;
            case 0xF: // SRL A
                SRL_IDX_R(A);
                break;
            }
            break;
        case 0x4:
            if ((t8 & 0x0F) < 0x08)
                BIT_x_IDX(0);
            else
                BIT_x_IDX(1);
            mCycleType = MCycleType::ALU;
            mCycleType = MCycleType::FETCH;
            idMode = IDMode::BASIC;
            intAlign = false;
            break;
        case 0x5:
            if ((t8 & 0x0F) < 0x08)
                BIT_x_IDX(2);
            else
                BIT_x_IDX(3);
            mCycleType = MCycleType::ALU;
            mCycleType = MCycleType::FETCH;
            idMode = IDMode::BASIC;
            intAlign = false;
            break;
        case 0x6:
            if ((t8 & 0x0F) < 0x08)
                BIT_x_IDX(4);
            else
                BIT_x_IDX(5);
            mCycleType = MCycleType::ALU;
            mCycleType = MCycleType::FETCH;
            idMode = IDMode::BASIC;
            intAlign = false;
            break;
        case 0x7:
            if ((t8 & 0x0F) < 0x08)
                BIT_x_IDX(6);
            else
                BIT_x_IDX(7);
            mCycleType = MCycleType::ALU;
            mCycleType = MCycleType::FETCH;
            idMode = IDMode::BASIC;
            intAlign = false;
            break;
        case 0x8:
            switch(t8 & 0x0F)
            {
            case 0x0: // RES 0,B
                RES_x_IDX(0, B);
                break;
            case 0x1: // RES 0,C
                RES_x_IDX(0, C);
                break;
            case 0x2: // RES 0,D
                RES_x_IDX(0, D);
                break;
            case 0x3: // RES 0,E
                RES_x_IDX(0, E);
                break;
            case 0x4: // RES 0,H
                RES_x_IDX(0, H);
                break;
            case 0x5: // RES 0,L
                RES_x_IDX(0, L);
                break;
            case 0x6: // RES 0,(HL)
                RES_x_IDX(0, t8);
                break;
            case 0x7: // RES 0,A
                RES_x_IDX(0, A);
                break;
            case 0x8: // RES 1,B
                RES_x_IDX(1, B);
                break;
            case 0x9: // RES 1,C
                RES_x_IDX(1, C);
                break;
            case 0xA: // RES 1,D
                RES_x_IDX(1, D);
                break;
            case 0xB: // RES 1,E
                RES_x_IDX(1, E);
                break;
            case 0xC: // RES 1,H
                RES_x_IDX(1, H);
                break;
            case 0xD: // RES 1,L
                RES_x_IDX(1, L);
                break;
            case 0xE: // RES 1,(HL)
                RES_x_IDX(1, t8);
                break;
            case 0xF: // RES 1,A
                RES_x_IDX(1, A);
                break;
            }
            break;
        case 0x9:
            switch(t8 & 0x0F)
            {
            case 0x0: // RES 2,B
                RES_x_IDX(2, B);
                break;
            case 0x1: // RES 2,C
                RES_x_IDX(2, C);
                break;
            case 0x2: // RES 2,D
                RES_x_IDX(2, D);
                break;
            case 0x3: // RES 2,E
                RES_x_IDX(2, E);
                break;
            case 0x4: // RES 2,H
                RES_x_IDX(2, H);
                break;
            case 0x5: // RES 2,L
                RES_x_IDX(2, L);
                break;
            case 0x6: // RES 2,(HL)
                RES_x_IDX(2, t8);
                break;
            case 0x7: // RES 2,A
                RES_x_IDX(2, A);
                break;
            case 0x8: // RES 3,B
                RES_x_IDX(3, B);
                break;
            case 0x9: // RES 3,C
                RES_x_IDX(3, C);
                break;
            case 0xA: // RES 3,D
                RES_x_IDX(3, D);
                break;
            case 0xB: // RES 3,E
                RES_x_IDX(3, E);
                break;
            case 0xC: // RES 3,H
                RES_x_IDX(3, H);
                break;
            case 0xD: // RES 3,L
                RES_x_IDX(3, L);
                break;
            case 0xE: // RES 3,(HL)
                RES_x_IDX(3, t8);
                break;
            case 0xF: // RES 3,A
                RES_x_IDX(3, A);
                break;
            }
            break;
        case 0xA:
            switch(t8 & 0x0F)
            {
            case 0x0: // RES 4,B
                RES_x_IDX(4, B);
                break;
            case 0x1: // RES 4,C
                RES_x_IDX(4, C);
                break;
            case 0x2: // RES 4,D
                RES_x_IDX(4, D);
                break;
            case 0x3: // RES 4,E
                RES_x_IDX(4, E);
                break;
            case 0x4: // RES 4,H
                RES_x_IDX(4, H);
                break;
            case 0x5: // RES 4,L
                RES_x_IDX(4, L);
                break;
            case 0x6: // RES 4,(HL)
                RES_x_IDX(4, t8);
                break;
            case 0x7: // RES 4,A
                RES_x_IDX(4, A);
                break;
            case 0x8: // RES 5,B
                RES_x_IDX(5, B);
                break;
            case 0x9: // RES 5,C
                RES_x_IDX(5, C);
                break;
            case 0xA: // RES 5,D
                RES_x_IDX(5, D);
                break;
            case 0xB: // RES 5,E
                RES_x_IDX(5, E);
                break;
            case 0xC: // RES 5,H
                RES_x_IDX(5, H);
                break;
            case 0xD: // RES 5,L
                RES_x_IDX(5, L);
                break;
            case 0xE: // RES 5,(HL)
                RES_x_IDX(5, t8);
                break;
            case 0xF: // RES 5,A
                RES_x_IDX(5, A);
                break;
            }
            break;
        case 0xB:
            switch(t8 & 0x0F)
            {
            case 0x0: // RES 6,B
                RES_x_IDX(6, B);
                break;
            case 0x1: // RES 6,C
                RES_x_IDX(6, C);
                break;
            case 0x2: // RES 6,D
                RES_x_IDX(6, D);
                break;
            case 0x3: // RES 6,E
                RES_x_IDX(6, E);
                break;
            case 0x4: // RES 6,H
                RES_x_IDX(6, H);
                break;
            case 0x5: // RES 6,L
                RES_x_IDX(6, L);
                break;
            case 0x6: // RES 6,(HL)
                RES_x_IDX(6, t8);
                break;
            case 0x7: // RES 6,A
                RES_x_IDX(6, A);
                break;
            case 0x8: // RES 7,B
                RES_x_IDX(7, B);
                break;
            case 0x9: // RES 7,C
                RES_x_IDX(7, C);
                break;
            case 0xA: // RES 7,D
                RES_x_IDX(7, D);
                break;
            case 0xB: // RES 7,E
                RES_x_IDX(7, E);
                break;
            case 0xC: // RES 7,H
                RES_x_IDX(7, H);
                break;
            case 0xD: // RES 7,L
                RES_x_IDX(7, L);
                break;
            case 0xE: // RES 7,(HL)
                RES_x_IDX(7, t8);
                break;
            case 0xF: // RES 7,A
                RES_x_IDX(7, A);
                break;
            }
            break;
        case 0xC:
            switch(t8 & 0x0F)
            {
            case 0x0: // SET 0,B
                SET_x_IDX(0, B);
                break;
            case 0x1: // SET 0,C
                SET_x_IDX(0, C);
                break;
            case 0x2: // SET 0,D
                SET_x_IDX(0, D);
                break;
            case 0x3: // SET 0,E
                SET_x_IDX(0, E);
                break;
            case 0x4: // SET 0,H
                SET_x_IDX(0, H);
                break;
            case 0x5: // SET 0,L
                SET_x_IDX(0, L);
                break;
            case 0x6: // SET 0,(HL)
                SET_x_IDX(0, t8);
                break;
            case 0x7: // SET 0,A
                SET_x_IDX(0, A);
                break;
            case 0x8: // SET 1,B
                SET_x_IDX(1, B);
                break;
            case 0x9: // SET 1,C
                SET_x_IDX(1, C);
                break;
            case 0xA: // SET 1,D
                SET_x_IDX(1, D);
                break;
            case 0xB: // SET 1,E
                SET_x_IDX(1, E);
                break;
            case 0xC: // SET 1,H
                SET_x_IDX(1, H);
                break;
            case 0xD: // SET 1,L
                SET_x_IDX(1, L);
                break;
            case 0xE: // SET 1,(HL)
                SET_x_IDX(1, t8);
                break;
            case 0xF: // SET 1,A
                SET_x_IDX(1, A);
                break;
            }
            break;
        case 0xD:
            switch(t8 & 0x0F)
            {
            case 0x0: // SET 2,B
                SET_x_IDX(2, B);
                break;
            case 0x1: // SET 2,C
                SET_x_IDX(2, C);
                break;
            case 0x2: // SET 2,D
                SET_x_IDX(2, D);
                break;
            case 0x3: // SET 2,E
                SET_x_IDX(2, E);
                break;
            case 0x4: // SET 2,H
                SET_x_IDX(2, H);
                break;
            case 0x5: // SET 2,L
                SET_x_IDX(2, L);
                break;
            case 0x6: // SET 2,(HL)
                SET_x_IDX(2, t8);
                break;
            case 0x7: // SET 2,A
                SET_x_IDX(2, A);
                break;
            case 0x8: // SET 3,B
                SET_x_IDX(3, B);
                break;
            case 0x9: // SET 3,C
                SET_x_IDX(3, C);
                break;
            case 0xA: // SET 3,D
                SET_x_IDX(3, D);
                break;
            case 0xB: // SET 3,E
                SET_x_IDX(3, E);
                break;
            case 0xC: // SET 3,H
                SET_x_IDX(3, H);
                break;
            case 0xD: // SET 3,L
                SET_x_IDX(3, L);
                break;
            case 0xE: // SET 3,(HL)
                SET_x_IDX(3, t8);
                break;
            case 0xF: // SET 3,A
                SET_x_IDX(3, A);
                break;        mCycleType = MCycleType::FETCH;
                idMode = IDMode::BASIC;
            }
            break;
        case 0xE:
            switch(t8 & 0x0F)
            {
            case 0x0: // SET 4,B
                SET_x_IDX(4, B);
                break;
            case 0x1: // SET 4,C
                SET_x_IDX(4, C);
                break;
            case 0x2: // SET 4,D
                SET_x_IDX(4, D);
                break;
            case 0x3: // SET 4,E
                SET_x_IDX(4, E);
                break;
            case 0x4: // SET 4,H
                SET_x_IDX(4, H);
                break;
            case 0x5: // SET 4,L
                SET_x_IDX(4, L);
                break;
            case 0x6: // SET 4,(HL)
                SET_x_IDX(4, t8);
                break;
            case 0x7: // SET 4,A
                SET_x_IDX(4, A);
                break;
            case 0x8: // SET 5,B
                SET_x_IDX(5, B);
                break;
            case 0x9: // SET 5,C
                SET_x_IDX(5, C);
                break;
            case 0xA: // SET 5,D
                SET_x_IDX(5, D);
                break;
            case 0xB: // SET 5,E
                SET_x_IDX(5, E);
                break;
            case 0xC: // SET 5,H
                SET_x_IDX(5, H);
                break;
            case 0xD: // SET 5,L
                SET_x_IDX(5, L);
                break;
            case 0xE: // SET 5,(HL)
                SET_x_IDX(5, t8);
                break;
            case 0xF: // SET 5,A
                SET_x_IDX(5, A);
                break;
            }
            break;
        case 0xF:
            switch(t8 & 0x0F)
            {
            case 0x0: // SET 6,B
                SET_x_IDX(6, B);
                break;
            case 0x1: // SET 6,C
                SET_x_IDX(6, C);
                break;
            case 0x2: // SET 6,D
                SET_x_IDX(6, D);
                break;
            case 0x3: // SET 6,E
                SET_x_IDX(6, E);
                break;
            case 0x4: // SET 6,H
                SET_x_IDX(6, H);
                break;
            case 0x5: // SET 6,L
                SET_x_IDX(6, L);
                break;
            case 0x6: // SET 6,(HL)
                SET_x_IDX(6, t8);
                break;
            case 0x7: // SET 6,A
                SET_x_IDX(6, A);
                break;
            case 0x8: // SET 7,B
                SET_x_IDX(7, B);
                break;
            case 0x9: // SET 7,C
                SET_x_IDX(7, C);
                break;
            case 0xA: // SET 7,D
                SET_x_IDX(7, D);
                break;
            case 0xB: // SET 7,E
                SET_x_IDX(7, E);
                break;
            case 0xC: // SET 7,H
                SET_x_IDX(7, H);
                break;
            case 0xD: // SET 7,L
                SET_x_IDX(7, L);
                break;
            case 0xE: // SET 7,(HL)
                SET_x_IDX(7, t8);
                break;
            case 0xF: // SET 7,A
                SET_x_IDX(7, A);
                break;
            }
            break;
        }
        DR = tByte;
        break;
    case 5:
        mCycleType = MCycleType::FETCH;
        idMode = IDMode::BASIC;
        break;
    }

}

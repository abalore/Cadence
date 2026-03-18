#include "Z80.h"

bool Z80::Step_CB()
{
    switch(IR >> 4)
    {
    case 0x0:
        switch(IR & 0x0F)
        {
        case 0x0: // RLC B
            RLC_R(B);
            break;
        case 0x1: // RLC C
            RLC_R(C);
            break;
        case 0x2: // RLC D
            RLC_R(D);
            break;
        case 0x3: // RLC E
            RLC_R(E);
            break;
        case 0x4: // RLC H
            RLC_R(H);
            break;
        case 0x5: // RLC L
            RLC_R(L);
            break;
        case 0x6: // RLC (HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0x7: // RLC A
            RLC_R(A);
            break;
        case 0x8: // RRC B
            RRC_R(B);
            break;
        case 0x9: // RRC C
            RRC_R(C);
            break;
        case 0xA: // RRC D
            RRC_R(D);
            break;
        case 0xB: // RRC E
            RRC_R(E);
            break;
        case 0xC: // RRC H
            RRC_R(H);
            break;
        case 0xD: // RRC L
            RRC_R(L);
            break;
        case 0xE: // RRC (HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0xF: // RRC A
            RRC_R(A);
            break;
        }
        break;
    case 0x1:
        switch(IR & 0x0F)
        {
        case 0x0: // RL B
            RL_R(B);
            break;
        case 0x1: // RL C
            RL_R(C);
            break;
        case 0x2: // RL D
            RL_R(D);
            break;
        case 0x3: // RL E
            RL_R(E);
            break;
        case 0x4: // RL H
            RL_R(H);
            break;
        case 0x5: // RL L
            RL_R(L);
            break;
        case 0x6: // RL (HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0x7: // RL A
            RL_R(A);
            break;
        case 0x8: // RR B
            RR_R(B);
            break;
        case 0x9: // RR C
            RR_R(C);
            break;
        case 0xA: // RR D
            RR_R(D);
            break;
        case 0xB: // RR E
            RR_R(E);
            break;
        case 0xC: // RR H
            RR_R(H);
            break;
        case 0xD: // RR L
            RR_R(L);
            break;
        case 0xE: // RR (HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0xF: // RR A
            RR_R(A);
            break;
        }
        break;
    case 0x2:
        switch(IR & 0x0F)
        {
        case 0x0: // SLA B
            SLA_R(B);
            break;
        case 0x1: // SLA C
            SLA_R(C);
            break;
        case 0x2: // SLA D
            SLA_R(D);
            break;
        case 0x3: // SLA E
            SLA_R(E);
            break;
        case 0x4: // SLA H
            SLA_R(H);
            break;
        case 0x5: // SLA L
            SLA_R(L);
            break;
        case 0x6: // SLA (HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0x7: // SLA A
            SLA_R(A);
            break;
        case 0x8: // SRA B
            SRA_R(B);
            break;
        case 0x9: // SRA C
            SRA_R(C);
            break;
        case 0xA: // SRA D
            SRA_R(D);
            break;
        case 0xB: // SRA E
            SRA_R(E);
            break;
        case 0xC: // SRA H
            SRA_R(H);
            break;
        case 0xD: // SRA L
            SRA_R(L);
            break;
        case 0xE: // SRA (HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0xF: // SRA A
            SRA_R(A);
            break;
        }
        break;
    case 0x3:
        switch(IR & 0x0F)
        {
        case 0x0: // SLL B
            SLL_R(B);
            break;
        case 0x1: // SLL C
            SLL_R(C);
            break;
        case 0x2: // SLL D
            SLL_R(D);
            break;
        case 0x3: // SLL E
            SLL_R(E);
            break;
        case 0x4: // SLL H
            SLL_R(H);
            break;
        case 0x5: // SLL L
            SLL_R(L);
            break;
        case 0x6: // SLL (HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0x7: // SLL A
            SLL_R(A);
            break;
        case 0x8: // SRL B
            SRL_R(B);
            break;
        case 0x9: // SRL C
            SRL_R(C);
            break;
        case 0xA: // SRL D
            SRL_R(D);
            break;
        case 0xB: // SRL E
            SRL_R(E);
            break;
        case 0xC: // SRL H
            SRL_R(H);
            break;
        case 0xD: // SRL L
            SRL_R(L);
            break;
        case 0xE: // SRL (HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0xF: // SRL A
            SRL_R(A);
            break;
        }
        break;
    case 0x4:
        switch(IR & 0x0F)
        {
        case 0x0: // BIT 0,B
            BIT_x_R(0, &B);
            break;
        case 0x1: // BIT 0,C
            BIT_x_R(0, &C);
            break;
        case 0x2: // BIT 0,D
            BIT_x_R(0, &D);
            break;
        case 0x3: // BIT 0,E
            BIT_x_R(0, &E);
            break;
        case 0x4: // BIT 0,H
            BIT_x_R(0, &H);
            break;
        case 0x5: // BIT 0,L
            BIT_x_R(0, &L);
            break;
        case 0x6: // BIT 0,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0x7: // BIT 0,A
            BIT_x_R(0, &A);
            break;
        case 0x8: // BIT 1,B
            BIT_x_R(1, &B);
            break;
        case 0x9: // BIT 1,C
            BIT_x_R(1, &C);
            break;
        case 0xA: // BIT 1,D
            BIT_x_R(1, &D);
            break;
        case 0xB: // BIT 1,E
            BIT_x_R(1, &E);
            break;
        case 0xC: // BIT 1,H
            BIT_x_R(1, &H);
            break;
        case 0xD: // BIT 1,L
            BIT_x_R(1, &L);
            break;
        case 0xE: // BIT 1,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0xF: // BIT 1,A
            BIT_x_R(1, &A);
            break;
        }
        break;
    case 0x5:
        switch(IR & 0x0F)
        {
        case 0x0: // BIT 2,B
            BIT_x_R(2, &B);
            break;
        case 0x1: // BIT 2,C
            BIT_x_R(2, &C);
            break;
        case 0x2: // BIT 2,D
            BIT_x_R(2, &D);
            break;
        case 0x3: // BIT 2,E
            BIT_x_R(2, &E);
            break;
        case 0x4: // BIT 2,H
            BIT_x_R(2, &H);
            break;
        case 0x5: // BIT 2,L
            BIT_x_R(2, &L);
            break;
        case 0x6: // BIT 2,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0x7: // BIT 2,A
            BIT_x_R(2, &A);
            break;
        case 0x8: // BIT 3,B
            BIT_x_R(3, &B);
            break;
        case 0x9: // BIT 3,C
            BIT_x_R(3, &C);
            break;
        case 0xA: // BIT 3,D
            BIT_x_R(3, &D);
            break;
        case 0xB: // BIT 3,E
            BIT_x_R(3, &E);
            break;
        case 0xC: // BIT 3,H
            BIT_x_R(3, &H);
            break;
        case 0xD: // BIT 3,L
            BIT_x_R(3, &L);
            break;
        case 0xE: // BIT 3,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0xF: // BIT 3,A
            BIT_x_R(3, &A);
            break;
        }
        break;
    case 0x6:
        switch(IR & 0x0F)
        {
        case 0x0: // BIT 4,B
            BIT_x_R(4, &B);
            break;
        case 0x1: // BIT 4,C
            BIT_x_R(4, &C);
            break;
        case 0x2: // BIT 4,D
            BIT_x_R(4, &D);
            break;
        case 0x3: // BIT 4,E
            BIT_x_R(4, &E);
            break;
        case 0x4: // BIT 4,H
            BIT_x_R(4, &H);
            break;
        case 0x5: // BIT 4,L
            BIT_x_R(4, &L);
            break;
        case 0x6: // BIT 4,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0x7: // BIT 4,A
            BIT_x_R(4, &A);
            break;
        case 0x8: // BIT 5,B
            BIT_x_R(5, &B);
            break;
        case 0x9: // BIT 5,C
            BIT_x_R(5, &C);
            break;
        case 0xA: // BIT 5,D
            BIT_x_R(5, &D);
            break;
        case 0xB: // BIT 5,E
            BIT_x_R(5, &E);
            break;
        case 0xC: // BIT 5,H
            BIT_x_R(5, &H);
            break;
        case 0xD: // BIT 5,L
            BIT_x_R(5, &L);
            break;
        case 0xE: // BIT 5,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0xF: // BIT 5,A
            BIT_x_R(5, &A);
            break;
        }
        break;
    case 0x7:
        switch(IR & 0x0F)
        {
        case 0x0: // BIT 6,B
            BIT_x_R(6, &B);
            break;
        case 0x1: // BIT 6,C
            BIT_x_R(6, &C);
            break;
        case 0x2: // BIT 6,D
            BIT_x_R(6, &D);
            break;
        case 0x3: // BIT 6,E
            BIT_x_R(6, &E);
            break;
        case 0x4: // BIT 6,H
            BIT_x_R(6, &H);
            break;
        case 0x5: // BIT 6,L
            BIT_x_R(6, &L);
            break;
        case 0x6: // BIT 6,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0x7: // BIT 6,A
            BIT_x_R(6, &A);
            break;
        case 0x8: // BIT 7,B
            BIT_x_R(7, &B);
            break;
        case 0x9: // BIT 7,C
            BIT_x_R(7, &C);
            break;
        case 0xA: // BIT 7,D
            BIT_x_R(7, &D);
            break;
        case 0xB: // BIT 7,E
            BIT_x_R(7, &E);
            break;
        case 0xC: // BIT 7,H
            BIT_x_R(7, &H);
            break;
        case 0xD: // BIT 7,L
            BIT_x_R(7, &L);
            break;
        case 0xE: // BIT 7,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0xF: // BIT 7,A
            BIT_x_R(7, &A);
            break;
        }
        break;
    case 0x8:
        switch(IR & 0x0F)
        {
        case 0x0: // RES 0,B
            RES_x_R(0, &B);
            break;
        case 0x1: // RES 0,C
            RES_x_R(0, &C);
            break;
        case 0x2: // RES 0,D
            RES_x_R(0, &D);
            break;
        case 0x3: // RES 0,E
            RES_x_R(0, &E);
            break;
        case 0x4: // RES 0,H
            RES_x_R(0, &H);
            break;
        case 0x5: // RES 0,L
            RES_x_R(0, &L);
            break;
        case 0x6: // RES 0,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0x7: // RES 0,A
            RES_x_R(0, &A);
            break;
        case 0x8: // RES 1,B
            RES_x_R(1, &B);
            break;
        case 0x9: // RES 1,C
            RES_x_R(1, &C);
            break;
        case 0xA: // RES 1,D
            RES_x_R(1, &D);
            break;
        case 0xB: // RES 1,E
            RES_x_R(1, &E);
            break;
        case 0xC: // RES 1,H
            RES_x_R(1, &H);
            break;
        case 0xD: // RES 1,L
            RES_x_R(1, &L);
            break;
        case 0xE: // RES 1,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0xF: // RES 1,A
            RES_x_R(1, &A);
            break;
        }
        break;
    case 0x9:
        switch(IR & 0x0F)
        {
        case 0x0: // RES 2,B
            RES_x_R(2, &B);
            break;
        case 0x1: // RES 2,C
            RES_x_R(2, &C);
            break;
        case 0x2: // RES 2,D
            RES_x_R(2, &D);
            break;
        case 0x3: // RES 2,E
            RES_x_R(2, &E);
            break;
        case 0x4: // RES 2,H
            RES_x_R(2, &H);
            break;
        case 0x5: // RES 2,L
            RES_x_R(2, &L);
            break;
        case 0x6: // RES 2,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0x7: // RES 2,A
            RES_x_R(2, &A);
            break;
        case 0x8: // RES 3,B
            RES_x_R(3, &B);
            break;
        case 0x9: // RES 3,C
            RES_x_R(3, &C);
            break;
        case 0xA: // RES 3,D
            RES_x_R(3, &D);
            break;
        case 0xB: // RES 3,E
            RES_x_R(3, &E);
            break;
        case 0xC: // RES 3,H
            RES_x_R(3, &H);
            break;
        case 0xD: // RES 3,L
            RES_x_R(3, &L);
            break;
        case 0xE: // RES 3,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0xF: // RES 3,A
            RES_x_R(3, &A);
            break;
        }
        break;
    case 0xA:
        switch(IR & 0x0F)
        {
        case 0x0: // RES 4,B
            RES_x_R(4, &B);
            break;
        case 0x1: // RES 4,C
            RES_x_R(4, &C);
            break;
        case 0x2: // RES 4,D
            RES_x_R(4, &D);
            break;
        case 0x3: // RES 4,E
            RES_x_R(4, &E);
            break;
        case 0x4: // RES 4,H
            RES_x_R(4, &H);
            break;
        case 0x5: // RES 4,L
            RES_x_R(4, &L);
            break;
        case 0x6: // RES 4,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0x7: // RES 4,A
            RES_x_R(4, &A);
            break;
        case 0x8: // RES 5,B
            RES_x_R(5, &B);
            break;
        case 0x9: // RES 5,C
            RES_x_R(5, &C);
            break;
        case 0xA: // RES 5,D
            RES_x_R(5, &D);
            break;
        case 0xB: // RES 5,E
            RES_x_R(5, &E);
            break;
        case 0xC: // RES 5,H
            RES_x_R(5, &H);
            break;
        case 0xD: // RES 5,L
            RES_x_R(5, &L);
            break;
        case 0xE: // RES 5,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0xF: // RES 5,A
            RES_x_R(5, &A);
            break;
        }
        break;
    case 0xB:
        switch(IR & 0x0F)
        {
        case 0x0: // RES 6,B
            RES_x_R(6, &B);
            break;
        case 0x1: // RES 6,C
            RES_x_R(6, &C);
            break;
        case 0x2: // RES 6,D
            RES_x_R(6, &D);
            break;
        case 0x3: // RES 6,E
            RES_x_R(6, &E);
            break;
        case 0x4: // RES 6,H
            RES_x_R(6, &H);
            break;
        case 0x5: // RES 6,L
            RES_x_R(6, &L);
            break;
        case 0x6: // RES 6,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0x7: // RES 6,A
            RES_x_R(6, &A);
            break;
        case 0x8: // RES 7,B
            RES_x_R(7, &B);
            break;
        case 0x9: // RES 7,C
            RES_x_R(7, &C);
            break;
        case 0xA: // RES 7,D
            RES_x_R(7, &D);
            break;
        case 0xB: // RES 7,E
            RES_x_R(7, &E);
            break;
        case 0xC: // RES 7,H
            RES_x_R(7, &H);
            break;
        case 0xD: // RES 7,L
            RES_x_R(7, &L);
            break;
        case 0xE: // RES 7,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0xF: // RES 7,A
            RES_x_R(7, &A);
            break;
        }
        break;
    case 0xC:
        switch(IR & 0x0F)
        {
        case 0x0: // SET 0,B
            SET_x_R(0, &B);
            break;
        case 0x1: // SET 0,C
            SET_x_R(0, &C);
            break;
        case 0x2: // SET 0,D
            SET_x_R(0, &D);
            break;
        case 0x3: // SET 0,E
            SET_x_R(0, &E);
            break;
        case 0x4: // SET 0,H
            SET_x_R(0, &H);
            break;
        case 0x5: // SET 0,L
            SET_x_R(0, &L);
            break;
        case 0x6: // SET 0,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0x7: // SET 0,A
            SET_x_R(0, &A);
            break;
        case 0x8: // SET 1,B
            SET_x_R(1, &B);
            break;
        case 0x9: // SET 1,C
            SET_x_R(1, &C);
            break;
        case 0xA: // SET 1,D
            SET_x_R(1, &D);
            break;
        case 0xB: // SET 1,E
            SET_x_R(1, &E);
            break;
        case 0xC: // SET 1,H
            SET_x_R(1, &H);
            break;
        case 0xD: // SET 1,L
            SET_x_R(1, &L);
            break;
        case 0xE: // SET 1,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0xF: // SET 1,A
            SET_x_R(1, &A);
            break;
        }
        break;
    case 0xD:
        switch(IR & 0x0F)
        {
        case 0x0: // SET 2,B
            SET_x_R(2, &B);
            break;
        case 0x1: // SET 2,C
            SET_x_R(2, &C);
            break;
        case 0x2: // SET 2,D
            SET_x_R(2, &D);
            break;
        case 0x3: // SET 2,E
            SET_x_R(2, &E);
            break;
        case 0x4: // SET 2,H
            SET_x_R(2, &H);
            break;
        case 0x5: // SET 2,L
            SET_x_R(2, &L);
            break;
        case 0x6: // SET 2,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0x7: // SET 2,A
            SET_x_R(2, &A);
            break;
        case 0x8: // SET 3,B
            SET_x_R(3, &B);
            break;
        case 0x9: // SET 3,C
            SET_x_R(3, &C);
            break;
        case 0xA: // SET 3,D
            SET_x_R(3, &D);
            break;
        case 0xB: // SET 3,E
            SET_x_R(3, &E);
            break;
        case 0xC: // SET 3,H
            SET_x_R(3, &H);
            break;
        case 0xD: // SET 3,L
            SET_x_R(3, &L);
            break;
        case 0xE: // SET 3,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0xF: // SET 3,A
            SET_x_R(3, &A);
            break;
        }
        break;
    case 0xE:
        switch(IR & 0x0F)
        {
        case 0x0: // SET 4,B
            SET_x_R(4, &B);
            break;
        case 0x1: // SET 4,C
            SET_x_R(4, &C);
            break;
        case 0x2: // SET 4,D
            SET_x_R(4, &D);
            break;
        case 0x3: // SET 4,E
            SET_x_R(4, &E);
            break;
        case 0x4: // SET 4,H
            SET_x_R(4, &H);
            break;
        case 0x5: // SET 4,L
            SET_x_R(4, &L);
            break;
        case 0x6: // SET 4,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0x7: // SET 4,A
            SET_x_R(4, &A);
            break;
        case 0x8: // SET 5,B
            SET_x_R(5, &B);
            break;
        case 0x9: // SET 5,C
            SET_x_R(5, &C);
            break;
        case 0xA: // SET 5,D
            SET_x_R(5, &D);
            break;
        case 0xB: // SET 5,E
            SET_x_R(5, &E);
            break;
        case 0xC: // SET 5,H
            SET_x_R(5, &H);
            break;
        case 0xD: // SET 5,L
            SET_x_R(5, &L);
            break;
        case 0xE: // SET 5,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0xF: // SET 5,A
            SET_x_R(5, &A);
            break;
        }
        break;
    case 0xF:
        switch(IR & 0x0F)
        {
        case 0x0: // SET 6,B
            SET_x_R(6, &B);
            break;
        case 0x1: // SET 6,C
            SET_x_R(6, &C);
            break;
        case 0x2: // SET 6,D
            SET_x_R(6, &D);
            break;
        case 0x3: // SET 6,E
            SET_x_R(6, &E);
            break;
        case 0x4: // SET 6,H
            SET_x_R(6, &H);
            break;
        case 0x5: // SET 6,L
            SET_x_R(6, &L);
            break;
        case 0x6: // SET 6,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0x7: // SET 6,A
            SET_x_R(6, &A);
            break;
        case 0x8: // SET 7,B
            SET_x_R(7, &B);
            break;
        case 0x9: // SET 7,C
            SET_x_R(7, &C);
            break;
        case 0xA: // SET 7,D
            SET_x_R(7, &D);
            break;
        case 0xB: // SET 7,E
            SET_x_R(7, &E);
            break;
        case 0xC: // SET 7,H
            SET_x_R(7, &H);
            break;
        case 0xD: // SET 7,L
            SET_x_R(7, &L);
            break;
        case 0xE: // SET 7,(HL)
            ShiftBitOpIndHL(IR);
            break;
        case 0xF: // SET 7,A
            SET_x_R(7, &A);
            break;
        }
        break;
    }
    return true;
}


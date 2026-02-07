#include "Headers/Z80.h"

void Z80::Step_IDX()
{
    switch(IR >> 4)
    {
    case 0x0:
        switch(IR & 0x0F)
        {
        case 0x4: // INC B
        case 0x5: // DEC B
        case 0x6: // LD B,n
        case 0xC: // INC C
        case 0xD: // DEC C
        case 0xE: // LD C,n
            Step_basic();
            break;
        case 0x9: // ADD IX,BC
            ADD_IDX_RR(BC);
            break;
        }
        break;
    case 0x1:
        switch(IR & 0x0F)
        {
        case 0x4: // INC D
        case 0x5: // DEC D
        case 0x6: // LD D,n
        case 0xC: // INC E
        case 0xD: // DEC E
        case 0xE: // LD E,n
            Step_basic();
            break;
        case 0x9: // ADD IX,DE
            ADD_IDX_RR(DE);
            break;
        }
        break;
    case 0x2:
        switch(IR & 0x0F)
        {
        case 0x1: // LD IX,nn
            LD_RR_nn(*IDX);
            break;
        case 0x2: // LD (nn),IX
            LD_Ind_RR(*IDX);
            break;
        case 0x3: // INC IX
            INC_RR(*IDX);
            break;
        case 0x4: // INC IXH
            INC_R(*(*IDX).H);
            break;
        case 0x5: // DEC IXH
            DEC_R(*(*IDX).H);
            break;
        case 0x6: // LD IXH,n
            LD_R_n(*(*IDX).H);
            break;
        case 0x9: // ADD IX,IX
            ADD_IDX_RR(*IDX);
            break;
        case 0xA: // SLD IX,(nn)
            LD_RR_Ind(*IDX);
            break;
        case 0xB: // DEC IX
            DEC_RR(*IDX);
            break;
        case 0xC: // INC IXL
            INC_R(*(*IDX).L);
            break;
        case 0xD: // DEC IXL
            DEC_R(*(*IDX).L);
            break;
        case 0xE: // LD IXL,n
            LD_R_n(*(*IDX).L);
            break;
        }
        break;
    case 0x3:
        switch(IR & 0x0F)
        {
        case 0x4: // INC (IX+d)
        case 0x5: // DEC (IX+d)
            Step_IDX_2();
            break;
        case 0x6: // LD (IX+d),n
            Step_IDX_3();
            break;
        case 0x9: // ADD IX,SP
            ADD_IDX_RR(SP);
            break;
        case 0xC: // INC A
        case 0xD: // DEC A
        case 0xE: // LD A,n
            Step_basic();
            break;
        }
        break;
    case 0x4:
        switch(IR & 0x0F)
        {
        case 0x0: //
        case 0x1: //
        case 0x2: //
        case 0x3: //
        case 0x7: //
        case 0x8: //
        case 0x9: //
        case 0xA: //
        case 0xB: //
        case 0xF: //
            Step_basic();
            break;
        case 0x4: // LD B,IDX.H
            B = *(*IDX).H;
            break;
        case 0x5: // LD B,IDX.L
            B = *(*IDX).L;
            break;
        case 0x6: // LD B,(IDX+n)
        case 0xE: // LD C,(IDX+n)
            Step_IDX_2();
            break;
        case 0xC: // LD C,IDX.H
            C = *(*IDX).H;
            break;
        case 0xD: // LD C,IDX.L
            C = *(*IDX).L;
            break;
        }
        break;
    case 0x5:
        switch(IR & 0x0F)
        {
        case 0x0: //
        case 0x1: //
        case 0x2: //
        case 0x3: //
        case 0x7: //
        case 0x8: //
        case 0x9: //
        case 0xA: //
        case 0xB: //
        case 0xF: //
            Step_basic();
            break;
        case 0x4: // LD D,IDX.H
            D = *(*IDX).H;
            break;
        case 0x5: // LD D,IDX.L
            D = *(*IDX).L;
            break;
        case 0x6: // LD D,(IDX+n)
        case 0xE: // LD E,(IDX+n)
            Step_IDX_2();
            break;
        case 0xC: // LD E,IDX.H
            E = *(*IDX).H;
            break;
        case 0xD: // LD E,IDX.L
            E = *(*IDX).L;
            break;
        }
        break;
    case 0x6:
        switch(IR & 0x0F)
        {
        case 0x0: // LD IDX.H,B
            *(*IDX).H = B;
            break;
        case 0x1: // LD IDX.H,C
            *(*IDX).H = C;
            break;
        case 0x2: // LD IDX.H,D
            *(*IDX).H = D;
            break;
        case 0x3: // LD IDX.H,E
            *(*IDX).H = E;
            break;
        case 0x4: // LD IDX.H,IDX.H
            break;
        case 0x5: // LD IDX.H,IDX.L
            *(*IDX).H = *(*IDX).L;
            break;
        case 0x6: // LD H,(IDX+n)
        case 0xE: // LD L,(IDX+n)
            Step_IDX_2();
            break;
        case 0x7: // LD IDX.H,A
            *(*IDX).H = A;
            break;
        case 0x8: // LD IDX.L,B
            *(*IDX).L = B;
            break;
        case 0x9: // LD IDX.L,C
            *(*IDX).L = C;
            break;
        case 0xA: // LD IDX.L,D
            *(*IDX).L = D;
            break;
        case 0xB: // LD IDX.L,E
            *(*IDX).L = E;
            break;
        case 0xC: // LD IDX.L,IDX.H
            *(*IDX).L = *(*IDX).H;
            break;
        case 0xD: // LD IDX.L,IDX.L
            break;
        case 0xF: // LD IDX.L,A
            *(*IDX).L = A;
            break;
        }
        break;
    case 0x7:
        switch(IR & 0x0F)
        {
        case 0x0: //
        case 0x1: //
        case 0x2: //
        case 0x3: //
        case 0x4: //
        case 0x5: //
        case 0x7: //
        case 0xE: // LD A,(IX+d)
            Step_IDX_2();
            break;
        case 0x8: //
        case 0x9: //
        case 0xA: //
        case 0xB: //
        case 0xF: //
            Step_basic();
            break;
        case 0xC: // LD A,IDX.H
            A = *(*IDX).H;
            break;
        case 0xD: // LD A,IDX.L
            A = *(*IDX).L;
            break;
        }
        break;
    case 0x8:
        switch(IR & 0x0F)
        {
        case 0x0: //
        case 0x1: //
        case 0x2: //
        case 0x3: //
        case 0x7: //
        case 0x8: //
        case 0x9: //
        case 0xA: //
        case 0xB: //
        case 0xF: //
            Step_basic();
            break;
        case 0x6: //
        case 0xE: //
            Step_IDX_2();
            break;
        case 0x4: // ADD IDX.H
            ADD_R(*(*IDX).H);
            break;
        case 0x5: // ADD IDX.L
            ADD_R(*(*IDX).L);
            break;
        case 0xC: // ADC IDX.H
            ADC_R(*(*IDX).H);
            break;
        case 0xD: // ADC IDX.L
            ADC_R(*(*IDX).L);
            break;
        }
        break;
    case 0x9:
        switch(IR & 0x0F)
        {
        case 0x0: //
        case 0x1: //
        case 0x2: //
        case 0x3: //
        case 0x7: //
        case 0x8: //
        case 0x9: //
        case 0xA: //
        case 0xB: //
        case 0xF: //
            Step_basic();
            break;
        case 0x6: //
        case 0xE: //
            Step_IDX_2();
            break;
        case 0x4: // SUB IDX.H
            SUB_R(*(*IDX).H);
            break;
        case 0x5: // SUB IDX.L
            SUB_R(*(*IDX).L);
            break;
        case 0xC: // SBC IDX.H
            SBC_R(*(*IDX).H);
            break;
        case 0xD: // SBC IDX.L
            SBC_R(*(*IDX).L);
            break;
        }
        break;
    case 0xA:
        switch(IR & 0x0F)
        {
        case 0x0: //
        case 0x1: //
        case 0x2: //
        case 0x3: //
        case 0x7: //
        case 0x8: //
        case 0x9: //
        case 0xA: //
        case 0xB: //
        case 0xF: //
            Step_basic();
            break;
        case 0x6: //
        case 0xE: //
            Step_IDX_2();
            break;
        case 0x4: // AND IDX.H
            AND_R(*(*IDX).H);
            break;
        case 0x5: // AND IDX.L
            AND_R(*(*IDX).L);
            break;
        case 0xC: // XOR IDX.H
            XOR_R(*(*IDX).H);
            break;
        case 0xD: // XOR IDX.L
            XOR_R(*(*IDX).L);
            break;
        }
        break;
    case 0xB:
        switch(IR & 0x0F)
        {
        case 0x0: //
        case 0x1: //
        case 0x2: //
        case 0x3: //
        case 0x7: //
        case 0x8: //
        case 0x9: //
        case 0xA: //
        case 0xB: //
        case 0xF: //
            Step_basic();
            break;
        case 0x6: //
        case 0xE: //
            Step_IDX_2();
            break;
        case 0x4: // OR IDX.H
            OR_R(*(*IDX).H);
            break;
        case 0x5: // OR IDX.L
            OR_R(*(*IDX).L);
            break;
        case 0xC: // CP IDX.H
            CP_R(*(*IDX).H);
            break;
        case 0xD: // CP IDX.L
            CP_R(*(*IDX).L);
            break;
        }
        break;
    case 0xC:
        switch(IR & 0x0F)
        {
        case 0xB: // IDX BIT OP
            Step_IDX_CB();
            idMode = IDMode::IDXBIT;
            return;
        }
        break;
    case 0xE:
        switch(IR & 0x0F)
        {
        case 0x1: // POP IDX
            /////////////////////////// Duda con NOPs
            POP_RR(*IDX);
            break;
        case 0x3: // EX SP,(IDX)
            EX_IDX_Ind_SP();
            break;
        case 0x5: // PUSH IDX
            PUSH_RR(*IDX);
            break;
        case 0x9: // JP (IDX)
            JP_IDX();
            break;
        }
        break;
    case 0xF:
        switch(IR & 0x0F)
        {
        case 0x9: // LD SP,IX
            LD_SP_IDX();
            break;
        }
        break;
    }
    if (mCycleType == MCycleType::FETCH)
        idMode = IDMode::BASIC;
}

#include "Z80.h"

bool Z80::Step_IDX()
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
            return Step_basic();
        case 0x9: // ADD IX,BC
            return ADD_IDX_RR(BC);
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
            return Step_basic();
        case 0x9: // ADD IX,DE
            return ADD_IDX_RR(DE);
        }
        break;
    case 0x2:
        switch(IR & 0x0F)
        {
        case 0x1: // LD IX,nn
            return LD_RR_nn(*IDX);
        case 0x2: // LD (nn),IX
            return LD_Ind_RR(*IDX);
        case 0x3: // INC IX
            return INC_RR(*IDX);
        case 0x4: // INC IXH
            INC_R(*(*IDX).H);
            break;
        case 0x5: // DEC IXH
            DEC_R(*(*IDX).H);
            break;
        case 0x6: // LD IXH,n
            return LD_R_n(*(*IDX).H);
        case 0x9: // ADD IX,IX
            return ADD_IDX_RR(*IDX);
        case 0xA: // LD IX,(nn)
            return LD_RR_Ind(*IDX);
        case 0xB: // DEC IX
            return DEC_RR(*IDX);
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
            return INC_Ind_IDX();
        case 0x5: // DEC (IX+d)
            return DEC_Ind_IDX();
        case 0x6: // LD (IX+d),n
            return Step_IDX_3();
        case 0x9: // ADD IX,SP
            return ADD_IDX_vv(SP);
        case 0xC: // INC A
        case 0xD: // DEC A
        case 0xE: // LD A,n
            return Step_basic();
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
            return Step_basic();
        case 0x4: // LD B,IDX.H
            B = *(*IDX).H;
            break;
        case 0x5: // LD B,IDX.L
            B = *(*IDX).L;
            break;
        case 0x6: // LD B,(IDX+n)
        case 0xE: // LD C,(IDX+n)
            return Step_IDX_2();
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
            return Step_basic();
        case 0x4: // LD D,IDX.H
            D = *(*IDX).H;
            break;
        case 0x5: // LD D,IDX.L
            D = *(*IDX).L;
            break;
        case 0x6: // LD D,(IDX+n)
        case 0xE: // LD E,(IDX+n)
            return Step_IDX_2();
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
            return Step_IDX_2();
        case 0x8: //
        case 0x9: //
        case 0xA: //
        case 0xB: //
        case 0xF: //
            return Step_basic();
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
            return Step_basic();
        case 0x6:
            return ADD_Ind_IDX();
        case 0xE:
            return ADC_Ind_IDX();
        case 0x4: // ADD IDX.H
            ADD_v(*(*IDX).H);
            break;
        case 0x5: // ADD IDX.L
            ADD_v(*(*IDX).L);
            break;
        case 0xC: // ADC IDX.H
            ADC_v(*(*IDX).H);
            break;
        case 0xD: // ADC IDX.L
            ADC_v(*(*IDX).L);
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
            return Step_basic();
        case 0x6:
            return SUB_Ind_IDX();
        case 0xE:
            return SBC_Ind_IDX();
        case 0x4: // SUB IDX.H
            SUB_v(*(*IDX).H);
            break;
        case 0x5: // SUB IDX.L
            SUB_v(*(*IDX).L);
            break;
        case 0xC: // SBC IDX.H
            SBC_v(*(*IDX).H);
            break;
        case 0xD: // SBC IDX.L
            SBC_v(*(*IDX).L);
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
            return Step_basic();
        case 0x6:
            return AND_Ind_IDX();
        case 0xE:
            return XOR_Ind_IDX();
        case 0x4: // AND IDX.H
            AND_v(*(*IDX).H);
            break;
        case 0x5: // AND IDX.L
            AND_v(*(*IDX).L);
            break;
        case 0xC: // XOR IDX.H
            XOR_v(*(*IDX).H);
            break;
        case 0xD: // XOR IDX.L
            XOR_v(*(*IDX).L);
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
            return Step_basic();
        case 0x6:
            return OR_Ind_IDX();
        case 0xE:
            return CP_Ind_IDX();
        case 0x4: // OR IDX.H
            OR_v(*(*IDX).H);
            break;
        case 0x5: // OR IDX.L
            OR_v(*(*IDX).L);
            break;
        case 0xC: // CP IDX.H
            CP_v(*(*IDX).H);
            break;
        case 0xD: // CP IDX.L
            CP_v(*(*IDX).L);
            break;
        }
        break;
    case 0xC:
        switch(IR & 0x0F)
        {
        case 0xB: // IDX BIT OP
            idMode = IDMode::IDXBIT;
            mCycle--;
            return false;
        }
        break;
    case 0xE:
        switch(IR & 0x0F)
        {
        case 0x1: // POP IDX
            /////////////////////////// Duda con NOPs
            return POP_RR(*IDX);
        case 0x3: // EX SP,(IDX)
            return EX_IDX_Ind_SP();
        case 0x5: // PUSH IDX
            return PUSH_RR(*IDX);
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
    return false;
}

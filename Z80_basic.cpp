#include "Z80.h"

bool Z80::Step_basic()
{
    switch(IR >> 4)
    {
    case 0x0:
        switch(IR & 0x0F)
        {
        case 0x0: // NOP  {4T}
            break;
        case 0x1: // LD BC,nn   {12T}
            return LD_RR_nn(BC);
        case 0x2: // LD (BC),A    {8T}
            return LD_Ind_RR_A(BC);
        case 0x3: // INC BC   {8T}
            return INC_RR(BC);
        case 0x4: // INC B  {4T}
            INC_R(B);
            break;
        case 0x5: // DEC B  {4T}
            DEC_R(B);
            break;
        case 0x6: // LD B,n  {8T}
            return LD_R_n(B);
        case 0x7: // RLCA  {4T}
            RLCA();
            break;
        case 0x8: // EX AF,AF'  {4T}
            EX_AF_AF_();
            break;
        case 0x9: // ADD HL,BC  {12T}
            return ADD_HL_vv(BC.Get());
        case 0xA: // LD A,(BC)    {8T}
            return LD_A_I_RR(BC);
        case 0xB: // DEC BC   {8T}
            return DEC_RR(BC);
        case 0xC: // INC C  {4T}
            INC_R(C);
            break;
        case 0xD: // DEC C  {4T}
            DEC_R(C);
            break;
        case 0xE: // LD C,n {8T}
            return LD_R_n(C);
        case 0xF: // RRCA   {4T}
            RRCA();
            break;
        }
        break;
    case 0x1:
        switch(IR & 0x0F)
        {
        case 0x0: // DJNZ d   {B>0 16T / B=0 8T}
            return DJNZ();
        case 0x1: // LD DE,nn   {12T}
            return LD_RR_nn(DE);
        case 0x2: // LD (DE),A    {8T}
            return LD_Ind_RR_A(DE);
        case 0x3: // INC DE   {8T}
            return INC_RR(DE);
        case 0x4: // INC D    {4T}
            INC_R(D);
            break;
        case 0x5: // DEC D    {4T}
            DEC_R(D);
            break;
        case 0x6: // LD D,n   {8T}
            return LD_R_n(D);
        case 0x7: // RLA    {4T}
            RLA();
            break;
        case 0x8: // JR d   {12T}
            return JR(true);
        case 0x9: // ADD HL,DE    {12T}
            return ADD_HL_vv(DE.Get());
        case 0xA: // LD A,(DE)    {8T}
            return LD_A_I_RR(DE);
        case 0xB: // DEC DE   {8T}
            return DEC_RR(DE);
        case 0xC: // INC E    {4T}
            INC_R(E);
            break;
        case 0xD: // DEC E    {4T}
            DEC_R(E);
            break;
        case 0xE: // LD E,n   {8T}
            return LD_R_n(E);
        case 0xF: // RRA    {4T}
            RRA();
            break;
        }
        break;
    case 0x2:
        switch(IR & 0x0F)
        {
        case 0x0: // JR NZ,d    {fZ 8T, !fZ 12T}
            return JR(!fZ);
        case 0x1: // LD HL,nn   {12T}
            return LD_RR_nn(HL);
        case 0x2: //  LD (nn),HL     {20T}
            return LD_Ind_RR(HL);
        case 0x3: // INC HL   {8T}
            return INC_RR(HL);
        case 0x4: // INC H    {4T}
            INC_R(H);
            break;
        case 0x5: // DEC H    {4T}
            DEC_R(H);
            break;
        case 0x6: // LD H,n   {8T}
            return LD_R_n(H);
        case 0x7: // DAA    {4T}
            DAA();
            break;
        case 0x8: // JR Z,d    {fZ 8T, !fZ 12T}
            return JR(fZ);
        case 0x9: // ADD HL,HL
            return ADD_HL_vv(HL.Get());
        case 0xA: // LD HL,(nn)
            return LD_RR_Ind(HL);
        case 0xB: // DEC HL
            return DEC_RR(HL);
        case 0xC: // INC L
            INC_R(L);
            break;
        case 0xD: // DEC L
            DEC_R(L);
            break;
        case 0xE: // LD L,n   {8T}
            return LD_R_n(L);
        case 0xF: // CPL
            CPL();
            break;
        }
        break;
    case 0x3:
        switch(IR & 0x0F)
        {
        case 0x0: // JR NC,D    {fC 8T, !fC 12T}
            return JR(!fC);
        case 0x1: // LD SP,nn   {12T}
            return LD_WW_nn(SP);
        case 0x2: // LD (nn),A    {16T}
            return LD_Ind_A();
        case 0x3: // INC SP   {8T}
            return INC_WW(SP);
        case 0x4: // INC (HL)
            return INC_Ind_HL();
        case 0x5: // DEC (HL)
            return DEC_Ind_HL();
        case 0x6: // LD (HL),n
            return LD_Ind_HL_n();
        case 0x7: // SCF
            SCF();
            break;
        case 0x8: // JR C,d
            return JR(fC);
        case 0x9: // ADD HL,SP
            return ADD_HL_vv(SP);
        case 0xA: // LD A,(nn)
            return LD_A_Ind();
        case 0xB: // DEC SP
            return DEC_WW(SP);
        case 0xC: // INC A
            INC_R(A);
            break;
        case 0xD: // DEC A
            DEC_R(A);
            break;
        case 0xE: // LD A,n
            return LD_R_n(A);
        case 0xF: // CCF
            CCF();
            break;
        }
        break;
    case 0x4:
        switch(IR & 0x0F)
        {
        case 0x0: // LD B,B
            break;
        case 0x1: // LD B,C
            B = C;
            break;
        case 0x2: // LD B,D
            B = D;
            break;
        case 0x3: // LD B,E
            B = E;
            break;
        case 0x4: // LD B,H
            B = H;
            break;
        case 0x5: // LD B,L
            B = L;
            break;
        case 0x6: // LD B,(HL)
            return LD_R_Ind_HL(B);
        case 0x7: // LD B,A
            B = A;
            break;
        case 0x8: // LD C,B
            C = B;
            break;
        case 0x9: // LD C,C
            break;
        case 0xA: // LD C,D
            C = D;
            break;
        case 0xB: // LD C,E
            C = E;
            break;
        case 0xC: // LD C,H
            C = H;
            break;
        case 0xD: // LD C,L
            C = L;
            break;
        case 0xE: // LD C,(HL)
            return LD_R_Ind_HL(C);
        case 0xF: // LD C,A
            C = A;
            break;
        }
        break;
    case 0x5:
        switch(IR & 0x0F)
        {
        case 0x0: // LD D,B
            D = B;
            break;
        case 0x1: // LD D,C
            D = C;
            break;
        case 0x2: // LD D,D
            break;
        case 0x3: // LD D,E
            D = E;
            break;
        case 0x4: // LD D,H
            D = H;
            break;
        case 0x5: // LD D,L
            D = L;
            break;
        case 0x6: // LD D,(HL)
            return LD_R_Ind_HL(D);
        case 0x7: // LD D,A
            D = A;
            break;
        case 0x8: // LD E,B
            E = B;
            break;
        case 0x9: // LD E,C
            E = C;
            break;
        case 0xA: // LD E,D
            E = D;
            break;
        case 0xB: // LD E,E
            break;
        case 0xC: // LD E,H
            E = H;
            break;
        case 0xD: // LD E,L
            E = L;
            break;
        case 0xE: // LD E,(HL)
            return LD_R_Ind_HL(E);
        case 0xF: // LD E,A
            E = A;
            break;
        }
        break;
    case 0x6:
        switch(IR & 0x0F)
        {
        case 0x0: // LD H,B
            H = B;
            break;
        case 0x1: // LD H,C
            H = C;
            break;
        case 0x2: // LD H,D
            H = D;
            break;
        case 0x3: // LD H,E
            H = E;
            break;
        case 0x4: // LD H,H
            break;
        case 0x5: // LD H,L
            H = L;
            break;
        case 0x6: // LD H,(HL)
            return LD_R_Ind_HL(H);
        case 0x7: // LD H,A
            H = A;
            break;
        case 0x8: // LD L,B
            L = B;
            break;
        case 0x9: // LD L,C
            L = C;
            break;
        case 0xA: // LD L,D
            L = D;
            break;
        case 0xB: // LD L,E
            L = E;
            break;
        case 0xC: // LD L,H
            L = H;
            break;
        case 0xD: // LD L,L
            break;
        case 0xE: // LD L,(HL)
            return LD_R_Ind_HL(L);
        case 0xF: // LD L,A
            L = A;
            break;
        }
        break;
    case 0x7:
        switch(IR & 0x0F)
        {
        case 0x0: // LD (HL),B
            return LD_Ind_HL_R(B);
        case 0x1: // LD (HL),C
            return LD_Ind_HL_R(C);
        case 0x2: // LD (HL),D
            return LD_Ind_HL_R(D);
        case 0x3: // LD (HL),E
            return LD_Ind_HL_R(E);
        case 0x4: // LD (HL),H
            return LD_Ind_HL_R(H);
        case 0x5: // LD (HL),L
            return LD_Ind_HL_R(L);
        case 0x6: // HALT
            Z80::halted = true;
            PC--;
            break;
        case 0x7: // LD (HL),A
            return LD_Ind_HL_R(A);
        case 0x8: // LD A,B
            A = B;
            break;
        case 0x9: // LD A,C
            A = C;
            break;
        case 0xA: // LD A,D
            A = D;
            break;
        case 0xB: // LD A,E
            A = E;
            break;
        case 0xC: // LD A,H
            A = H;
            break;
        case 0xD: // LD A,L
            A = L;
            break;
        case 0xE: // LD A,(HL)
            return LD_R_Ind_HL(A);
        case 0xF: // LD A,A
            break;
        }
        break;
    case 0x8:
        switch(IR & 0x0F)
        {
        case 0x0: // ADD A,B
            ADD_v(B);
            break;
        case 0x1: // ADD A,C
            ADD_v(C);
            break;
        case 0x2: // ADD A,D
            ADD_v(D);
            break;
        case 0x3: // ADD A,E
            ADD_v(E);
            break;
        case 0x4: // ADD A,H
            ADD_v(H);
            break;
        case 0x5: // ADD A,L
            ADD_v(L);
            break;
        case 0x6: // ADD A,(HL)
            return ADD_Ind_HL();
        case 0x7: // ADD A,A
            ADD_v(A);
            break;
        case 0x8: // ADC A,B
            ADC_v(B);
            break;
        case 0x9: // ADC A,C
            ADC_v(C);
            break;
        case 0xA: // ADC A,D
            ADC_v(D);
            break;
        case 0xB: // ADC A,E
            ADC_v(E);
            break;
        case 0xC: // ADC A,H
            ADC_v(H);
            break;
        case 0xD: // ADC A,L
            ADC_v(L);
            break;
        case 0xE: // ADC A,(HL)
            return ADC_Ind_HL();
        case 0xF: // ADC A,A
            ADC_v(A);
            break;
        }
        break;
    case 0x9:
        switch(IR & 0x0F)
        {
        case 0x0: // SUB B
            SUB_v(B);
            break;
        case 0x1: // SUB C
            SUB_v(C);
            break;
        case 0x2: // SUB D
            SUB_v(D);
            break;
        case 0x3: // SUB E
            SUB_v(E);
            break;
        case 0x4: // SUB H
            SUB_v(H);
            break;
        case 0x5: // SUB L
            SUB_v(L);
            break;
        case 0x6: // SUB (HL)
            return SUB_Ind_HL();
        case 0x7: // SUB A
            SUB_v(A);
            break;
        case 0x8: // SBC B
            SBC_v(B);
            break;
        case 0x9: // SBC C
            SBC_v(C);
            break;
        case 0xA: // SBC D
            SBC_v(D);
            break;
        case 0xB: // SBC E
            SBC_v(E);
            break;
        case 0xC: // SBC H
            SBC_v(H);
            break;
        case 0xD: // SBC L
            SBC_v(L);
            break;
        case 0xE: // SBC (HL)
            return SBC_Ind_HL();
        case 0xF: // SBC A
            SBC_v(A);
            break;
        }
        break;
    case 0xA:
        switch(IR & 0x0F)
        {
        case 0x0: // AND B
            AND_v(B);
            break;
        case 0x1: // AND C
            AND_v(C);
            break;
        case 0x2: // AND D
            AND_v(D);
            break;
        case 0x3: // AND e
            AND_v(E);
            break;
        case 0x4: // AND H
            AND_v(H);
            break;
        case 0x5: // AND L
            AND_v(L);
            break;
        case 0x6: // AND (HL)
            return AND_Ind_HL();
        case 0x7: // AND A
            AND_v(A);
            break;
        case 0x8: // XOR B
            XOR_v(B);
            break;
        case 0x9: // XOR c
            XOR_v(C);
            break;
        case 0xA: // XOR D
            XOR_v(D);
            break;
        case 0xB: // XOR E
            XOR_v(E);
            break;
        case 0xC: // XOR H
            XOR_v(H);
            break;
        case 0xD: // XOR L
            XOR_v(L);
            break;
        case 0xE: // XOR (HL)
            return XOR_Ind_HL();
        case 0xF: // XOR A
            XOR_v(A);
            break;
        }
        break;
    case 0xB:
        switch(IR & 0x0F)
        {
        case 0x0: // OR B
            OR_v(B);
            break;
        case 0x1: // OR C
            OR_v(C);
            break;
        case 0x2: // OR D
            OR_v(D);
            break;
        case 0x3: // OR E
            OR_v(E);
            break;
        case 0x4: // OR H
            OR_v(H);
            break;
        case 0x5: // OR L
            OR_v(L);
            break;
        case 0x6: // OR (HL)
            return OR_Ind_HL();
        case 0x7: // OR A
            OR_v(A);
            break;
        case 0x8: // CP B
            CP_v(B);
            break;
        case 0x9: // CP C
            CP_v(C);
            break;
        case 0xA: // CP D
            CP_v(D);
            break;
        case 0xB: // CP E
            CP_v(E);
            break;
        case 0xC: // CP H
            CP_v(H);
            break;
        case 0xD: // CP L
            CP_v(L);
            break;
        case 0xE: // CP (HL)
            return CP_Ind_HL();
        case 0xF: // CP A
            CP_v(A);
            break;
        }
        break;
    case 0xC:
        switch(IR & 0x0F)
        {
        case 0x0: // RET NZ
            return RET(!fZ);
        case 0x1: // POP BC
            return POP_RR(BC);
        case 0x2: // JP NZ,nn
            return JP(!fZ);
        case 0x3: // JP nn
            return JP(true);
        case 0x4: // CALL NZ,nn
            return CALL(!fZ);
        case 0x5: // PUSH BC
            return PUSH_RR(BC);
        case 0x6: // ADD A,n
            return ADD_n();
        case 0x7: // RST 00h
            return RST(0);
        case 0x8: // RET z
            return RET(fZ);
        case 0x9: // RET
            return RET();
        case 0xA: // JP Z,nn
            return JP(fZ);
        case 0xB: // BIT OP
            idMode = IDMode::BIT;
            mCycle--;
            return false;
        case 0xC: // CALL Z,nn
            return CALL(fZ);
        case 0xD: // CALL nn
            return CALL(true);
        case 0xE: // ADC A,n
            return ADC_n();
        case 0xF: // RST 08h
            return RST(0x08);
        }
        break;
    case 0xD:
        switch(IR & 0x0F)
        {
        case 0x0: // RET NC
            return RET(!fC);
        case 0x1: // POP DE
            return POP_RR(DE);
        case 0x2: // JP NC,NN
            return JP(!fC);
        case 0x3: // OUT (n),A
            return OUT_n_A();
        case 0x4: // CALL NC,nn
            return CALL(!fC);
        case 0x5: // PUSH_DE
            return PUSH_RR(DE);
        case 0x6: // SUB n
            return SUB_n();
        case 0x7: // RST 10h
            return RST(0x10);
        case 0x8: // RET C
            return RET(fC);
        case 0x9: // EXX
            EXX();
            break;
        case 0xA: // JP C,nn
            return JP(fC);
        case 0xB: // IN A,(n)
            return IN_A_n();
        case 0xC: // CALL C,nn
            return CALL(fC);
            break;
        case 0xD: // IX OP
            IDX = &IX;
            idMode = IDMode::IDX;
            mCycle--;
            return false;
        case 0xE: // SBC A,n
            return SBC_n();
        case 0xF: // RST 18h
            return RST(0x18);
        }
        break;
    case 0xE:
        switch(IR & 0x0F)
        {
        case 0x0: // RET PO
            return RET(!fP);
        case 0x1: // POP HL
            return POP_RR(HL);
        case 0x2: // JP PO,nn
            return JP(!fP);
        case 0x3: // EX (SP),HL
            return EX_HL_Ind_SP();
        case 0x4: // CALL PO,nn
            return CALL(!fP);
        case 0x5: // PUSH_HL
            return PUSH_RR(HL);
        case 0x6: // AND n
            return AND_n();
        case 0x7: // RST 20h
            return RST(0x20);
        case 0x8: // RET PE
            return RET(fP);
        case 0x9: // JP (HL)
            JP_HL();
            break;
        case 0xA: // JP PE,nn
            return JP(fP);
        case 0xB: // EX DE,HL
            EX_DE_HL();
            break;
        case 0xC: // CALL PE,nn
            return CALL(fP);
        case 0xD: // MISC
            idMode = IDMode::MISC;
            mCycle--;
            return false;
        case 0xE: // XOR n
            return XOR_n();
        case 0xF: // RST 28h
            return RST(0x28);
        }
        break;
    case 0xF:
        switch(IR & 0x0F)
        {
        case 0x0: // RET P
            return RET(!fS);
        case 0x1: // POP AF
            tC = POP_RR(AF);
            if (tC)
                DecodeF();
            return tC;
        case 0x2: // JP P,nn
            return JP(!fS);
        case 0x3: // DI
            IFF1 = false;
            IFF2 = false;
            break;
        case 0x4: // CALL P,nn
            return CALL(!fS);
        case 0x5: // PUSH_AF
            EncodeF();
            return PUSH_RR(AF);
        case 0x6: // OR n
            return OR_n();
        case 0x7: // RST 30h
            return RST(0x30);
        case 0x8: // RET M
            return RET(fS);
        case 0x9: // LD SP,HL
            return LD_SP_HL();
        case 0xA: // JP M,nn
            return JP(fS);
        case 0xB: // EI
            Z80::EIRequest = true;
            break;
        case 0xC: // CALL M,nn
            return CALL(fS);
        case 0xD: // IY op
            IDX = &IY;
            idMode = IDMode::IDX;
            mCycle--;
            return false;
        case 0xE: // CP n
            return CP_n();
        case 0xF: // RST 38h
            return RST(0x38);
        }
        break;
    }
    return true; // For simple instructions
}

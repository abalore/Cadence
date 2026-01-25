#include "Headers/Z80.h"

void Z80::Step_basic()
{
    switch(IR >> 4)
    {
    case 0x0:
        switch(IR & 0x0F)
        {
        case 0x0: // NOP  {4T}
            break;
        case 0x1: // LD BC,nn   {12T}
            LD_RR_nn(BC);
            break;
        case 0x2: // LD (BC),A    {8T}
            LD_Ind_RR_A(BC);
            break;
        case 0x3: // INC BC   {8T}
            INC_RR(BC);
            break;
        case 0x4: // INC B  {4T}
            INC_R(B);
            break;
        case 0x5: // DEC B  {4T}
            DEC_R(B);
            break;
        case 0x6: // LD B,n  {8T}
            LD_R_n(B);
            break;
        case 0x7: // RLCA  {4T}
            RLCA();
            break;
        case 0x8: // EX AF,AF'  {4T}
            EX_AF_AF_();
            break;
        case 0x9: // ADD HL,BC  {12T}
            ADD_HL_RR(BC);
            break;
        case 0xA: // LD A,(BC)    {8T}
            LD_A_I_RR(BC);
            break;
        case 0xB: // DEC BC   {8T}
            DEC_RR(BC);
            break;
        case 0xC: // INC C  {4T}
            INC_R(C);
            break;
        case 0xD: // DEC C  {4T}
            DEC_R(C);
            break;
        case 0xE: // LD C,n {8T}
            LD_R_n(C);
            break;
        case 0xF: // RRCA   {4T}
            RRCA();
            break;
        }
        break;
    case 0x1:
        switch(IR & 0x0F)
        {
        case 0x0: // DJNZ d   {B>0 16T / B=0 8T}
            DJNZ();
            break;
        case 0x1: // LD DE,nn   {12T}
            LD_RR_nn(DE);
            break;
        case 0x2: // LD (DE),A    {8T}
            LD_Ind_RR_A(DE);
            break;
        case 0x3: // INC DE   {8T}
            INC_RR(DE);
            break;
        case 0x4: // INC D    {4T}
            INC_R(D);
            break;
        case 0x5: // DEC D    {4T}
            DEC_R(D);
            break;
        case 0x6: // LD D,n   {8T}
            LD_R_n(D);
            break;
        case 0x7: // RLA    {4T}
            RLA();
            break;
        case 0x8: // JR d   {12T}
            JR(true);
            break;
        case 0x9: // ADD HL,DE    {12T}
            ADD_HL_RR(DE);
            break;
        case 0xA: // LD A,(DE)    {8T}
            LD_A_I_RR(DE);
            break;
        case 0xB: // DEC DE   {8T}
            DEC_RR(DE);
            break;
        case 0xC: // INC E    {4T}
            INC_R(E);
            break;
        case 0xD: // DEC E    {4T}
            DEC_R(E);
            break;
        case 0xE: // LD E,n   {8T}
            LD_R_n(E);
            break;
        case 0xF: // RRA    {4T}
            RRA();
            break;
        }
        break;
    case 0x2:
        switch(IR & 0x0F)
        {
        case 0x0: // JR NZ,d    {fZ 8T, !fZ 12T}
            JR(!fZ.Get());
            break;
        case 0x1: // LD HL,nn   {12T}
            LD_RR_nn(HL);
            break;
        case 0x2: //  LD (nn),HL     {20T}
            LD_Ind_RR(HL);
            break;
        case 0x3: // INC HL   {8T}
            INC_RR(HL);
            break;
        case 0x4: // INC H    {4T}
            INC_R(H);
            break;
        case 0x5: // DEC H    {4T}
            DEC_R(H);
            break;
        case 0x6: // LD H,n   {8T}
            LD_R_n(H);
            break;
        case 0x7: // DAA    {4T}
            DAA();
            break;
        case 0x8: // JR Z,d    {fZ 8T, !fZ 12T}
            JR(fZ.Get());
            break;
        case 0x9: // ADD HL,HL
            ADD_HL_RR(HL);
            break;
        case 0xA: // LD HL,(nn)
            LD_RR_Ind(HL);
            break;
        case 0xB: // DEC HL
            DEC_RR(HL);
            break;
        case 0xC: // INC L
            INC_R(L);
            break;
        case 0xD: // DEC L
            DEC_R(L);
            break;
        case 0xE: // LD L,n   {8T}
            LD_R_n(L);
            break;
        case 0xF: // CPL
            CPL();
            break;
        }
        break;
    case 0x3:
        switch(IR & 0x0F)
        {
        case 0x0: // JR NC,D    {fC 8T, !fC 12T}
            JR(!fC.Get());
            break;
        case 0x1: // LD SP,nn   {12T}
            LD_RR_nn(SP);
            break;
        case 0x2: // LD (nn),A    {16T}
            LD_Ind_A();
            break;
        case 0x3: // INC SP   {8T}
            INC_RR(SP);
            break;
        case 0x4: // INC (HL)
            INC_Ind_HL();
            break;
        case 0x5: // DEC (HL)
            DEC_Ind_HL();
            break;
        case 0x6: // LD (HL),n
            LD_Ind_HL_n();
            break;
        case 0x7: // SCF
            SCF();
            break;
        case 0x8: // JR C,d
            JR(fC.Get());
            break;
        case 0x9: // ADD HL,SP
            ADD_HL_RR(SP);
            break;
        case 0xA: // LD A,(nn)
            LD_A_Ind();
            break;
        case 0xB: // DEC SP
            DEC_RR(SP);
            break;
        case 0xC: // INC A
            INC_R(A);
            break;
        case 0xD: // DEC A
            DEC_R(A);
            break;
        case 0xE: // LD A,n
            LD_R_n(A);
            break;
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
            LD_R_Ind_HL(B);
            break;
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
            LD_R_Ind_HL(C);
            break;
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
            LD_R_Ind_HL(D);
            break;
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
            LD_R_Ind_HL(E);
            break;
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
            LD_R_Ind_HL(H);
            break;
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
            LD_R_Ind_HL(L);
            break;
        case 0xF: // LD L,A
            L = A;
            break;
        }
        break;
    case 0x7:
        switch(IR & 0x0F)
        {
        case 0x0: // LD (HL),B
            LD_Ind_HL_R(B);
            break;
        case 0x1: // LD (HL),C
            LD_Ind_HL_R(C);
            break;
        case 0x2: // LD (HL),D
            LD_Ind_HL_R(D);
            break;
        case 0x3: // LD (HL),E
            LD_Ind_HL_R(E);
            break;
        case 0x4: // LD (HL),H
            LD_Ind_HL_R(H);
            break;
        case 0x5: // LD (HL),L
            LD_Ind_HL_R(L);
            break;
        case 0x6: // HALT
            ////////////////////////////////////////////////////////////////////////////
            break;
        case 0x7: // LD (HL),A
            LD_Ind_HL_R(A);
            break;
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
            LD_R_Ind_HL(A);
            break;
        case 0xF: // LD A,A
            break;
        }
        break;
    case 0x8:
        switch(IR & 0x0F)
        {
        case 0x0: // ADD A,B
            ADD_R(B);
            break;
        case 0x1: // ADD A,C
            ADD_R(C);
            break;
        case 0x2: // ADD A,D
            ADD_R(D);
            break;
        case 0x3: // ADD A,E
            ADD_R(E);
            break;
        case 0x4: // ADD A,H
            ADD_R(H);
            break;
        case 0x5: // ADD A,L
            ADD_R(L);
            break;
        case 0x6: // ADD A,(HL)
            ADD_Ind_HL();
            break;
        case 0x7: // ADD A,A
            ADD_R(A);
            break;
        case 0x8: // ADC A,B
            ADC_R(B);
            break;
        case 0x9: // ADC A,C
            ADC_R(C);
            break;
        case 0xA: // ADC A,D
            ADC_R(D);
            break;
        case 0xB: // ADC A,E
            ADC_R(E);
            break;
        case 0xC: // ADC A,H
            ADC_R(H);
            break;
        case 0xD: // ADC A,L
            ADC_R(L);
            break;
        case 0xE: // ADC A,(HL)
            ADC_Ind_HL();
            break;
        case 0xF: // ADC A,A
            ADC_R(A);
            break;
        }
        break;
    case 0x9:
        switch(IR & 0x0F)
        {
        case 0x0: // SUB B
            SUB_R(B);
            break;
        case 0x1: // SUB C
            SUB_R(C);
            break;
        case 0x2: // SUB D
            SUB_R(D);
            break;
        case 0x3: // SUB E
            SUB_R(E);
            break;
        case 0x4: // SUB H
            SUB_R(H);
            break;
        case 0x5: // SUB L
            SUB_R(L);
            break;
        case 0x6: // SUB (HL)
            SUB_Ind_HL();
            break;
        case 0x7: // SUB A
            SUB_R(A);
            break;
        case 0x8: // SBC B
            SBC_R(B);
            break;
        case 0x9: // SBC C
            SBC_R(C);
            break;
        case 0xA: // SBC D
            SBC_R(D);
            break;
        case 0xB: // SBC E
            SBC_R(E);
            break;
        case 0xC: // SBC H
            SBC_R(H);
            break;
        case 0xD: // SBC L
            SBC_R(L);
            break;
        case 0xE: // SBC (HL)
            SBC_Ind_HL();
            break;
        case 0xF: // SBC A
            SBC_R(A);
            break;
        }
        break;
    case 0xA:
        switch(IR & 0x0F)
        {
        case 0x0: // AND B
            AND_R(B);
            break;
        case 0x1: // AND C
            AND_R(C);
            break;
        case 0x2: // AND D
            AND_R(D);
            break;
        case 0x3: // AND e
            AND_R(E);
            break;
        case 0x4: // AND H
            AND_R(H);
            break;
        case 0x5: // AND L
            AND_R(L);
            break;
        case 0x6: // AND (HL)
            AND_Ind_HL();
            break;
        case 0x7: // AND A
            AND_R(A);
            break;
        case 0x8: // XOR B
            XOR_R(B);
            break;
        case 0x9: // XOR c
            XOR_R(C);
            break;
        case 0xA: // XOR D
            XOR_R(D);
            break;
        case 0xB: // XOR E
            XOR_R(E);
            break;
        case 0xC: // XOR H
            XOR_R(H);
            break;
        case 0xD: // XOR L
            XOR_R(L);
            break;
        case 0xE: // XOR (HL)
            XOR_Ind_HL();
            break;
        case 0xF: // XOR A
            XOR_R(A);
            break;
        }
        break;
    case 0xB:
        switch(IR & 0x0F)
        {
        case 0x0: // OR B
            OR_R(B);
            break;
        case 0x1: // OR C
            OR_R(C);
            break;
        case 0x2: // OR D
            OR_R(D);
            break;
        case 0x3: // OR E
            OR_R(E);
            break;
        case 0x4: // OR H
            OR_R(H);
            break;
        case 0x5: // OR L
            OR_R(L);
            break;
        case 0x6: // OR (HL)
            OR_Ind_HL();
            break;
        case 0x7: // OR A
            OR_R(A);
            break;
        case 0x8: // CP B
            CP_R(B);
            break;
        case 0x9: // CP C
            CP_R(C);
            break;
        case 0xA: // CP D
            CP_R(D);
            break;
        case 0xB: // CP E
            CP_R(E);
            break;
        case 0xC: // CP H
            CP_R(H);
            break;
        case 0xD: // CP L
            CP_R(L);
            break;
        case 0xE: // CP (HL)
            CP_Ind_HL();
            break;
        case 0xF: // CP A
            CP_R(A);
            break;
        }
        break;
    case 0xC:
        switch(IR & 0x0F)
        {
        case 0x0: // RET NZ
            RET(!fZ.Get());
            break;
        case 0x1: // POP BC
            POP_RR(BC);
            break;
        case 0x2: // JP NZ,nn
            JP(!fZ.Get());
            break;
        case 0x3: // JP nn
            JP(true);
            break;
        case 0x4: // CALL NZ,nn
            CALL(!fZ.Get());
            break;
        case 0x5: // PUSH BC
            PUSH_RR(BC);
            break;
        case 0x6: // ADD A,n
            ADD_n();
            break;
        case 0x7: // RST 00h
            RST(0);
            break;
        case 0x8: // RET z
            RET(fZ.Get());
            break;
        case 0x9: // RET
            RET(true);
            break;
        case 0xA: // JP Z,nn
            JP(fZ.Get());
            break;
        case 0xB: // BIT OP //////////////////////////////////////
            idMode = IDMode::BIT;
            break;
        case 0xC: // CALL Z,nn
            CALL(fZ.Get());
            break;
        case 0xD: // CALL nn
            CALL(true);
            break;
        case 0xE: // ADC A,n
            ADC_n();
            break;
        case 0xF: // RST 08h
            RST(0x08);
            break;
        }
        break;
    case 0xD:
        switch(IR & 0x0F)
        {
        case 0x0: // RET NC
            RET(!fC.Get());
            break;
        case 0x1: // POP DE
            POP_RR(DE);
            break;
        case 0x2: // JP NC,NN
            JP(!fC.Get());
            break;
        case 0x3: // OUT (n),A
            //////////////////////////////////////////////////
            break;
        case 0x4: // CALL NC,nn
            CALL(!fC.Get());
            break;
        case 0x5: // PUSH_DE
            PUSH_RR(DE);
            break;
        case 0x6: // SUB n
            SUB_n();
            break;
        case 0x7: // RST 10h
            RST(0x10);
            break;
        case 0x8: // RET C
            RET(fC.Get());
            break;
        case 0x9: // EXX
            EXX();
            break;
        case 0xA: // JP C,nn
            JP(fC.Get());
            break;
        case 0xB: // IN A,(n)
            //////////////////////////////////////////////////////////
            break;
        case 0xC: // CALL C,nn
            CALL(fC.Get());
            break;
        case 0xD: // IX OP
            IDX = &IX;
            idMode = IDMode::IDX;
            break;
        case 0xE: // SBC A,n
            SBC_n();
            break;
        case 0xF: // RST 18h
            RST(0x18);
            break;
        }
        break;
    case 0xE:
        switch(IR & 0x0F)
        {
        case 0x0: // RET PO
            RET(!fP.Get());
            break;
        case 0x1: // POP HL
            POP_RR(HL);
            break;
        case 0x2: // JP PO,nn
            JP(!fP.Get());
            break;
        case 0x3: // EX (SP),HL
            EX_HL_Ind_SP();
            break;
        case 0x4: // CALL PO,nn
            CALL(!fP.Get());
            break;
        case 0x5: // PUSH_HL
            PUSH_RR(HL);
            break;
        case 0x6: // AND n
            AND_n();
            break;
        case 0x7: // RST 20h
            RST(0x20);
            break;
        case 0x8: // RET PE
            RET(fP.Get());
            break;
        case 0x9: // JP (HL)
            JP_HL();
            break;
        case 0xA: // JP PE,nn
            JP(fP.Get());
            break;
        case 0xB: // EX DE,HL
            EX_DE_HL();
            break;
        case 0xC: // CALL PE,nn
            CALL(fP.Get());
            break;
        case 0xD: // MISC
            idMode = IDMode::MISC;
            break;
        case 0xE: // XOR n
            XOR_n();
            break;
        case 0xF: // RST 28h
            RST(0x28);
            break;
        }
        break;
    case 0xF:
        switch(IR & 0x0F)
        {
        case 0x0: // RET P
            RET(!fS.Get());
            break;
        case 0x1: // POP AF
            POP_RR(AF);
            break;
        case 0x2: // JP P,nn
            JP(!fS.Get());
            break;
        case 0x3: // DI
            InterruptEnable = false;
            break;
        case 0x4: // CALL P,nn
            CALL(!fS.Get());
            break;
        case 0x5: // PUSH_AF
            PUSH_RR(AF);
            break;
        case 0x6: // OR n
            OR_n();
            break;
        case 0x7: // RST 30h
            RST(0x30);
            break;
        case 0x8: // RET M
            RET(fS.Get());
            break;
        case 0x9: // LD SP,HL
            SP = HL;
            break;
        case 0xA: // JP M,nn
            JP(fS.Get());
            break;
        case 0xB: // EI
            InterruptEnable = true;
            break;
        case 0xC: // CALL M,nn
            CALL(fS.Get());
            break;
        case 0xD: // IY op
            IDX = &IY;
            idMode = IDMode::IDX;
            break;
        case 0xE: // CP n
            CP_n();
            break;
        case 0xF: // RST 38h
            RST(0x38);
            break;
        }
        break;
    }
}

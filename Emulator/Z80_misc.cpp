#include "Headers/Z80.h"

void Z80::Step_misc()
{
    switch(IR >> 4)
    {
    case 0x4:
        switch(IR & 0x0F)
        {
        case 0x0: // IN B,(C)
            IN_R_PortBC(B);
            break;
        case 0x1: // OUT (C),B
            OUT_PortBC_R(B);
            break;
        case 0x2: // SBC HL,BC
            SBC_HL_RR(BC);
            break;
        case 0x3: // LD (nn),BC
            LD_Ind_RR(BC);
            break;
        case 0x4: // NEG
            NEG();
            break;
        case 0x5: // RETN
            RET(true);
            ////////////////////////////////////////////////////////
            break;
        case 0x6: // IM 0
            IM(0);
            break;
        case 0x7: // LD I,A
            LD_I_A();
            break;
        case 0x8: // IN C,(C)
            IN_R_PortBC(C);
            break;
        case 0x9: // OUT (C),C
            OUT_PortBC_R(C);
            break;
        case 0xA: // ADC HL,BC
            ADC_HL_RR(BC);
            break;
        case 0xB: // LD BC,(nn)
            LD_RR_Ind(BC);
            break;
        case 0xC: // NEG
            NEG();
            break;
        case 0xD: // RETI
            RET(true);
            /////////////////////////////////////////////////////////
            break;
        case 0xE: // IM 0
            /////////////////////////////////////////////////////////
            break;
        case 0xF: // LD R,A
            LD_R_A();
            break;
        }
        break;
    case 0x5:
        switch(IR & 0x0F)
        {
        case 0x0: // IN D,(C)
            IN_R_PortBC(D);
            break;
        case 0x1: // OUT (C),D
            OUT_PortBC_R(D);
            break;
        case 0x2: // SBC HL,DE
            SBC_HL_RR(DE);
            break;
        case 0x3: // LD (nn),DE
            LD_Ind_RR(DE);
            break;
        case 0x4: // NEG
            NEG();
            break;
        case 0x5: // RETN
            RET(true);
            ////////////////////////////////////////////////////////
            break;
        case 0x6: // IM 1
            IM(1);
            break;
        case 0x7: // LD A,I
            LD_A_I();
            break;
        case 0x8: // IN E,(C)
            IN_R_PortBC(E);
            break;
        case 0x9: // OUT (C),E
            OUT_PortBC_R(E);
            break;
        case 0xA: // ADC HL,DE
            ADC_HL_RR(DE);
            break;
        case 0xB: // LD DE,(nn)
            LD_RR_Ind(DE);
            break;
        case 0xC: // NEG
            NEG();
            break;
        case 0xD: // RETN
            RET(true);
            /////////////////////////////////////////////////////////
            break;
        case 0xE: // IM 2
            IM(2);
            break;
        case 0xF: // LD A,R
            LD_A_R();
            break;
        }
        break;
    case 0x6:
        switch(IR & 0x0F)
        {
        case 0x0: // IN H,(C)
            IN_R_PortBC(H);
            break;
        case 0x1: // OUT (C),H
            OUT_PortBC_R(H);
            break;
        case 0x2: // SBC HL,HL
            SBC_HL_RR(HL);
            break;
        case 0x3: // LD (nn),HL
            LD_Ind_RR(HL);
            break;
        case 0x4: // NEG
            NEG();
            break;
        case 0x5: // RETN
            RET(true);
            ////////////////////////////////////////////////////////
            break;
        case 0x6: // IM 0
            IM(0);
            break;
        case 0x7: // RRD
            RRD();
            break;
        case 0x8: // IN L,(C)
            IN_R_PortBC(L);
            break;
        case 0x9: // OUT (C),L
            OUT_PortBC_R(L);
            break;
        case 0xA: // ADC HL,HL
            ADC_HL_RR(HL);
            break;
        case 0xB: // LD HL,(nn)
            LD_RR_Ind(HL);
            break;
        case 0xC: // NEG
            NEG();
            break;
        case 0xD: // RETN
            RET(true);
            /////////////////////////////////////////////////////////
            break;
        case 0xE: // IM 0
            IM(0);
            break;
        case 0xF: // RLD
            RLD();
            break;
        }
        break;
    case 0x7:
        switch(IR & 0x0F)
        {
        case 0x0: // IN (C) Flags only
            IN_R_PortBC(t8);
            break;
        case 0x1: // OUT (C),0
            t8 = 0;
            OUT_PortBC_R(t8);
            break;
        case 0x2: // SBC HL,SP
            SBC_HL_RR(SP);
            break;
        case 0x3: // LD (nn),SP
            LD_Ind_RR(SP);
            break;
        case 0x4: // NEG
            NEG();
            break;
        case 0x5: // RETN
            RET(true);
            ////////////////////////////////////////////////////////
            break;
        case 0x6: // IM 1
            IM(1);
            break;
        case 0x7: // LD I,A
            LD_I_A();
            break;
        case 0x8: // IN A,(C)
            IN_R_PortBC(A);
            break;
        case 0x9: // OUT (C),A
            OUT_PortBC_R(A);
            break;
        case 0xA: // ADC HL,SP
            ADC_HL_RR(SP);
            break;
        case 0xB: // LD SP,(nn)
            LD_RR_Ind(SP);
            break;
        case 0xC: // NEG
            NEG();
            break;
        case 0xD: // RETN
            RET(true);
            /////////////////////////////////////////////////////////
            break;
        case 0xE: // IM 2
            IM(2);
            break;
        }
        break;
    case 0xA:
        switch(IR & 0x0F)
        {
        case 0x0: // LDI
            LDI(false);
            break;
        case 0x1: // CPI
            CPI(false);
            break;
        case 0x2: // INI
            /////////////////////////////////////
            break;
        case 0x3: // OUTI
            /////////////////////////////////////
            break;
        case 0x8: // LDD
            LDD(false);
            break;
        case 0x9: // CPD
            CPD(false);
            break;
        case 0xA: // IND
            //////////////////////////////////////
            break;
        case 0xB: // OUTD
            //////////////////////////////////////
            break;
        }
        break;
    case 0xB:
        switch(IR & 0x0F)
        {
        case 0x0: // LDIR
            LDI(true);
            break;
        case 0x1: // CPIR
            CPI(true);
            break;
        case 0x2: // INIR
            //////////////////////////////////////
            break;
        case 0x3: // OUTIR
            //////////////////////////////////////
            break;
        case 0x8: // LDDR
            LDD(true);
            break;
        case 0x9: // CPDR
            CPD(true);
            break;
        case 0xA: // INDR
            //////////////////////////////////////
            break;
        case 0xB: // OUTDR
            //////////////////////////////////////
            break;
        }
        break;
    }
    if (mCycleType == MCycleType::FETCH)
        idMode = IDMode::BASIC;
}

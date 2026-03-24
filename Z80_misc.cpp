#include "Z80.h"

bool Z80::Step_misc()
{
    switch(IR >> 4)
    {
    case 0x4:
        switch(IR & 0x0F)
        {
        case 0x0: // IN B,(C)
            return IN_R_PortBC(B);
        case 0x1: // OUT (C),B
            return OUT_PortBC_R(B);
        case 0x2: // SBC HL,BC
            return SBC_HL_vv(BC.Get());
        case 0x3: // LD (nn),BC
            return LD_Ind_RR(BC);
        case 0x4: // NEG
            NEG();
            break;
        case 0x5: // RETN
            IFF1 = IFF2;
            return RET();
        case 0x6: // IM 0
            IM(0);
            break;
        case 0x7: // LD I,A
            return LD_I_A();
        case 0x8: // IN C,(C)
            return IN_R_PortBC(C);
        case 0x9: // OUT (C),C
            return OUT_PortBC_R(C);
        case 0xA: // ADC HL,BC
            return ADC_HL_vv(BC.Get());
        case 0xB: // LD BC,(nn)
            return LD_RR_Ind(BC);
        case 0xC: // NEG
            NEG();
            break;
        case 0xD: // RETI
            return RET();
        case 0xE: // IM 0
            IM(0);
            break;
        case 0xF: // LD R,A
            return LD_R_A();
        }
        break;
    case 0x5:
        switch(IR & 0x0F)
        {
        case 0x0: // IN D,(C)
            return IN_R_PortBC(D);
        case 0x1: // OUT (C),D
            return OUT_PortBC_R(D);
        case 0x2: // SBC HL,DE
            return SBC_HL_vv(DE.Get());
        case 0x3: // LD (nn),DE
            return LD_Ind_RR(DE);
        case 0x4: // NEG
            NEG();
            break;
        case 0x5: // RETN
            IFF1 = IFF2;
            return RET();
        case 0x6: // IM 1
            IM(1);
            break;
        case 0x7: // LD A,I
            return LD_A_I();
        case 0x8: // IN E,(C)
            return IN_R_PortBC(E);
        case 0x9: // OUT (C),E
            return OUT_PortBC_R(E);
        case 0xA: // ADC HL,DE
            return ADC_HL_vv(DE.Get());
        case 0xB: // LD DE,(nn)
            return LD_RR_Ind(DE);
        case 0xC: // NEG
            NEG();
            break;
        case 0xD: // RETN
            IFF1 = IFF2;
            return RET();
        case 0xE: // IM 2
            IM(2);
            break;
        case 0xF: // LD A,R
            return LD_A_R();
        }
        break;
    case 0x6:
        switch(IR & 0x0F)
        {
        case 0x0: // IN H,(C)
            return IN_R_PortBC(H);
        case 0x1: // OUT (C),H
            return OUT_PortBC_R(H);
        case 0x2: // SBC HL,HL
            return SBC_HL_vv(HL.Get());
        case 0x3: // LD (nn),HL
            return LD_Ind_RR(HL);
        case 0x4: // NEG
            NEG();
            break;
        case 0x5: // RETN
            IFF1 = IFF2;
            return RET();
        case 0x6: // IM 0
            IM(0);
            break;
        case 0x7: // RRD
            return RRD();
        case 0x8: // IN L,(C)
            return IN_R_PortBC(L);
        case 0x9: // OUT (C),L
            return OUT_PortBC_R(L);
        case 0xA: // ADC HL,HL
            return ADC_HL_vv(HL.Get());
        case 0xB: // LD HL,(nn)
            return LD_RR_Ind(HL);
        case 0xC: // NEG
            NEG();
            break;
        case 0xD: // RETN
            IFF1 = IFF2;
            return RET();
        case 0xE: // IM 0
            IM(0);
            break;
        case 0xF: // RLD
            return RLD();
        }
        break;
    case 0x7:
        switch(IR & 0x0F)
        {
        case 0x0: // IN (C) Flags only
            return IN_R_PortBC(t8);
        case 0x1: // OUT (C),0
            t8 = 0;
            return OUT_PortBC_R(t8);
        case 0x2: // SBC HL,SP
            return SBC_HL_vv(SP);
        case 0x3: // LD (nn),SP
            return LD_Ind_WW(SP);
        case 0x4: // NEG
            NEG();
            break;
        case 0x5: // RETN
            IFF1 = IFF2;
            return RET();
        case 0x6: // IM 1
            IM(1);
            break;
        case 0x7: // LD I,A
            return LD_I_A();
        case 0x8: // IN A,(C)
            return IN_R_PortBC(A);
        case 0x9: // OUT (C),A
            return OUT_PortBC_R(A);
        case 0xA: // ADC HL,SP
            return ADC_HL_vv(SP);
        case 0xB: // LD SP,(nn)
            return LD_WW_Ind(SP);
        case 0xC: // NEG
            NEG();
            break;
        case 0xD: // RETN
            IFF1 = IFF2;
            return RET();
        case 0xE: // IM 2
            IM(2);
            break;
        }
        break;
    case 0xA:
        switch(IR & 0x0F)
        {
        case 0x0: // LDI
            return LDI(false);
        case 0x1: // CPI
            return CPI(false);
        case 0x2: // INI
            return INI(false, true);
        case 0x3: // OUTI
            return OUTI(false, true);
        case 0x8: // LDD
            return LDD(false);
        case 0x9: // CPD
            return CPD(false);
        case 0xA: // IND
            return INI(false, false);
        case 0xB: // OUTD
            return OUTI(false, false);
        }
        break;
    case 0xB:
        switch(IR & 0x0F)
        {
        case 0x0: // LDIR
            return LDI(true);
        case 0x1: // CPIR
            return CPI(true);
        case 0x2: // INIR
            return INI(true, true);
        case 0x3: // OUTIR
            return OUTI(true, true);
        case 0x8: // LDDR
            return LDD(true);
        case 0x9: // CPDR
            return CPD(true);
        case 0xA: // INDR
            return INI(true, false);
        case 0xB: // OUTDR
            return OUTI(true, false);
        }
        break;
    }
    return true; // For simple instructions
}

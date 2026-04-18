#ifndef Z80_H
#define Z80_H

#include "defs.h"
#include "Reg16.h"

enum MCycleType
{
    FETCH,
    READ,
    WRITE,
    IN,
    OUT,
    ALU3,
    ALU4,
    ALU5,
    INT,
    RELADDR,
    WRITEI,
    WRITED,
    READ4,
    READ5,
    WRITE5,
    WRITE4
};

enum IDMode
{
    BASIC,
    MISC,
    BIT,
    IDX,
    IDXBIT,
    INTEXEC
};

class Z80
{
public:
    static void Reset();
    static void Clock();
    static void Clock2();
    static void IRQ();
    static BYTE tCycle;
    static BYTE mCycle;
    static bool MREQ, IORQ, RD, WR;
    static word PC, SP;
    static Reg16 AF, BC, DE, HL, IX, IY;
    static Reg16 AF_, BC_, DE_, HL_;
    static bool fS, fZ, fH, fP, fN, fC, f3, f5;
    static BYTE A, F, B, C, D, E, H, L, IXH, IXL, IYH, IYL;
    static BYTE A_, F_, B_, C_, D_, E_, H_, L_;
    static BYTE t16H, t16L, SPH, SPL;
    static BYTE I, R;
    static BYTE IR;
    static BYTE DR;
    static word AR;
    static IDMode idMode;
    static MCycleType mCycleType;
    static bool IFF1;
    static bool IFF2;
    static bool InterruptRequest;
    static bool EIRequest;
    static bool stopPoint;
    static bool halted;
    static BYTE im;
    static dword nops;
    static bool M1;
    static bool WAIT;

    static void EncodeF();

private:
    static void RunMCycle();
    static void RunTCycle();

    static bool Step_basic();
    static bool Step_misc();
    static bool Step_CB();
    static bool Step_IDX();
    static bool Step_IDX_2();
    static bool Step_IDX_3();
    static bool Step_IDX_CB();
    static bool Step_Int_Exec();
    static bool ProcessINT();
    static bool ProcessFETCH();
    static bool ProcessREAD();
    static bool ProcessREAD4();
    static bool ProcessREAD5();
    static bool ProcessWRITE();
    static bool ProcessWRITE4();
    static bool ProcessWRITE5();
    static bool ProcessWRITEI();
    static bool ProcessWRITED();
    static bool ProcessIN();
    static bool ProcessOUT();
    static bool ProcessRELADDR();
    static bool ProcessALU(BYTE length);
    static void FinishInstruction();
    static BYTE t8;
    static Reg16 t16, tt16;
    static sbyte index;
    static BYTE tByte;
    static BYTE opCode;
    static Reg16 *IDX;
    static BYTE t_cp;
    static bool tC;
    static int tCV;
    static BYTE t;
    static word w1, w2, w3;
    static int i1, i2, i3;
    static short s1;
    static BYTE intACK;
    static bool lastMCycle;
    static bool lastTCycle;

    // 8 bit arithmetic and logic for A
    static void NEG();
    static void DAA();
    static void CPL();
    static void ADD_v(BYTE v);
    static void ADC_v(BYTE v);
    static void SUB_v(BYTE v);
    static void SBC_v(BYTE v);
    static void CP_v(BYTE v);
    static void AND_v(BYTE v);
    static void XOR_v(BYTE v);
    static void OR_v(BYTE v);
    static void INC_R(BYTE &reg);
    static void DEC_R(BYTE &reg);
    static bool ADD_n();
    static bool ADC_n();
    static bool SUB_n();
    static bool SBC_n();
    static bool AND_n();
    static bool XOR_n();
    static bool OR_n();
    static bool CP_n();
    // 8 bit arithmetic and logic for (HL)
    static bool ADD_Ind_HL();
    static bool ADC_Ind_HL();
    static bool SUB_Ind_HL();
    static bool SBC_Ind_HL();
    static bool CP_Ind_HL();
    static bool AND_Ind_HL();
    static bool XOR_Ind_HL();
    static bool OR_Ind_HL();
    static bool INC_Ind_HL();
    static bool DEC_Ind_HL();
    // 8 bit arithmetic and logic for (IDX+d)
    static bool ADD_Ind_IDX();
    static bool ADC_Ind_IDX();
    static bool SUB_Ind_IDX();
    static bool SBC_Ind_IDX();
    static bool CP_Ind_IDX();
    static bool AND_Ind_IDX();
    static bool XOR_Ind_IDX();
    static bool OR_Ind_IDX();
    static bool INC_Ind_IDX();
    static bool DEC_Ind_IDX();
    // 16 bit arithmetic
    static bool ADD_HL_vv(word w);
    static bool ADC_HL_vv(word w);
    static bool SBC_HL_vv(word w);
    static bool ADD_IDX_vv(word w);
    static bool INC_WW(word &w);
    static bool DEC_WW(word &w);
    static bool INC_RR(Reg16 reg);
    static bool DEC_RR(Reg16 reg);
    static bool ADD_IDX_RR(Reg16 reg);
    // Shift
    static void RLCA();
    static void RRCA();
    static void RLA();
    static void RRA();
    static void RLC_R(BYTE &reg);
    static void RRC_R(BYTE &reg);
    static void RL_R(BYTE &reg);
    static void RR_R(BYTE &reg);
    static void SLA_R(BYTE &reg);
    static void SRA_R(BYTE &reg);
    static void SLL_R(BYTE &reg);
    static void SRL_R(BYTE &reg);
    static bool ShiftOpIndHL(BYTE opCode);
    static void RLC_IDX_R(BYTE &reg);
    static void RRC_IDX_R(BYTE &reg);
    static void RL_IDX_R(BYTE &reg);
    static void RR_IDX_R(BYTE &reg);
    static void SLA_IDX_R(BYTE &reg);
    static void SRA_IDX_R(BYTE &reg);
    static void SLL_IDX_R(BYTE &reg);
    static void SRL_IDX_R(BYTE &reg);
    static bool RLD();
    static bool RRD();
    // Bitwise op
    static void BIT_x_R(int X, BYTE * R);
    static void RES_x_R(int X, BYTE * R);
    static void SET_x_R(int X, BYTE * R);
    static bool BIT_x_Ind_HL(int X);
    static bool RES_x_Ind_HL(int X);
    static bool SET_x_Ind_HL(int X);
    static void BIT_x_IDX(int X);
    static void RES_x_IDX(int X, BYTE &reg);
    static void SET_x_IDX(int X, BYTE &reg);
    // 8 bit load
    static bool LD_Ind_A();
    static bool LD_A_Ind();
    static bool LD_R_Ind_HL(BYTE &reg);
    static bool LD_Ind_HL_R(BYTE &reg);
    static bool LD_R_n(BYTE &reg);
    static bool LD_Ind_RR_A(Reg16 reg);
    static bool LD_A_I();
    static bool LD_I_A();
    static bool LD_A_R();
    static bool LD_R_A();
    // 16 bit load
    static bool LD_RR_addr(Reg16 reg);
    static bool LD_Ind_RR(Reg16 reg);
    static bool LD_RR_Ind(Reg16 reg);
    static bool LD_RR_nn(Reg16 reg);
    static bool LD_SP_HL();
    static bool LD_SP_IDX();
    static bool LD_WW_nn(word &w);
    static bool LD_Ind_WW(word w);
    static bool LD_WW_Ind(word &w);
    static bool LD_A_Ind_RR(Reg16 reg);
    static bool LD_Ind_HL_n();
    static bool PUSH_RR(Reg16 reg);
    static bool POP_RR(Reg16 reg);
    // Jump
    static bool JR(bool condition);
    static bool JP(bool condition);
    static bool CALL(bool condition);
    static bool RST(BYTE address);
    static bool RET();
    static bool RET(bool condition);
    static void JP_HL();
    static void JP_IDX();
    static bool DJNZ();
    // Block
    static bool LDI(bool R);
    static bool CPI(bool R);
    static bool LDD(bool R);
    static bool CPD(bool R);
    static bool INI(bool R, bool dir);
    static bool OUTI(bool R, bool dir);
    // Interchange
    static void EX_AF_AF_();
    static void EX_DE_HL();
    static void EXX();
    static bool EX_HL_Ind_SP();
    static bool EX_IDX_Ind_SP();
    // I/O
    static bool IN_R_PortBC(BYTE &reg);
    static bool IN_A_n();
    static bool OUT_PortBC_R(BYTE &reg);
    static bool OUT_n_A();
    // Control
    static void SCF();
    static void CCF();
    static void IM(int mode);
    // Helpers
    static bool GetParity(BYTE b);

    static void DecodeF();
    static void SetFlagsAfterShiftOp(BYTE b);
    static constexpr BYTE ExtendedM1[256] =
    {
        0,0,0,2,0,0,0,0,0,0,0,2,0,0,0,0,
        1,0,0,2,0,0,0,0,0,0,0,2,0,0,0,0,
        0,0,0,2,0,0,0,0,0,0,0,2,0,0,0,0,
        0,0,0,2,0,0,0,0,0,0,0,2,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,3,3,0,0,0,0,0,0,3,3,0,0,0,0,
        0,0,3,3,0,0,0,0,0,0,3,3,0,0,0,0,
        1,0,0,0,0,1,0,1,1,0,0,0,0,0,0,1,
        1,0,0,0,0,1,0,1,1,0,0,0,0,0,0,1,
        1,0,0,0,0,1,0,1,1,0,0,0,0,0,0,1,
        1,0,0,0,0,1,0,1,1,2,0,0,0,0,0,1
    };
};

#endif // Z80_H

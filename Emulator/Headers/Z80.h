#ifndef Z80_H
#define Z80_H

#include "defs.h"
#include "Reg16.h"
#include "Flag.h"

using namespace std;

enum MCycleType
{
    FETCH,
    READ,
    WRITE,
    IN,
    OUT,
    ALU,
    INT
};

enum IDMode
{
    BASIC,
    MISC,
    BIT,
    IDX,
    IDX2,
    IDXBIT
};

class Z80
{
public:
    static void Init();
    static void Reset();
    static void ClockEdge();
    static BYTE tCycle;
    static BYTE mCycle;
    static bool MREQ;
    static bool RD;
    static bool WR;
    static bool IORQ;
    // public static bool WAIT;
    static bool RFSH;
    static bool M1;
    static word PC;
    static Reg16 SP, AF, BC, DE, HL, IX, IY;
    static Reg16 AF_, BC_, DE_, HL_;
    static Flag fS, fZ, fH, fP, fN, fC, f3, f5;
    static BYTE A, F, B, C, D, E, H, L, IXH, IXL, IYH, IYL;
    static BYTE A_, F_, B_, C_, D_, E_, H_, L_;
    static BYTE t16H, t16L, SPH, SPL;
    static BYTE I, R;
    static BYTE IR;
    static BYTE DR;
    static IDMode idMode;
    static MCycleType mCycleType;
    static bool InterruptEnable;
    static bool InterruptRequest;
    static bool CLK;
private:
    static void Step();;
    static void Step_basic();
    static void Step_misc();
    static void Step_CB();
    static void Step_IDX();
    static void Step_IDX_2();
    static void Step_IDX_3();
    static void Step_IDX_CB();
    static void ProcessINT();
    static void ProcessFETCH();
    static void ProcessREAD();
    static void ProcessWRITE();
    static void ProcessIN();
    static void ProcessOUT();
    static void FinishInstruction();
    static BYTE t8;
    static Reg16 t16;
    static sbyte index;
    static BYTE tByte;
    static BYTE opCode;
    static Reg16 *IDX;
    static word tAddr;

    static void INC_R(BYTE &reg);
    static void DEC_R(BYTE &reg);
    static void JR(bool condition);
    static void LD_RR_addr(Reg16 reg);
    static void LD_Ind_RR(Reg16 reg);
    static void LD_RR_Ind(Reg16 reg);
    static void LD_Ind_A();
    static void LD_A_Ind();
    static void LD_R_Ind_HL(BYTE &reg);
    static void LD_Ind_HL_R(BYTE &reg);
    static void LD_RR_nn(Reg16 reg);
    static void INC_RR(Reg16 reg);
    static void DEC_RR(Reg16 reg);
    static void LD_R_n(BYTE &reg);
    static void LD_Ind_RR_A(Reg16 reg);
    static void PUSH_RR(Reg16 reg);
    static void POP_RR(Reg16 reg);
    static void ADD_HL_RR(Reg16 reg);
    static void ADD_A_v(BYTE v);
    static void ADD_n();
    static void ADC_n();
    static void SUB_n();
    static void SBC_n();
    static void AND_n();
    static void XOR_n();
    static void OR_n();
    static void CP_n();
    static void ADD_R(BYTE &reg);
    static void ADD_Ind_HL();
    static void ADC_Ind_HL();
    static void SUB_Ind_HL();
    static void SBC_Ind_HL();
    static void AND_Ind_HL();
    static void XOR_Ind_HL();
    static void OR_Ind_HL();
    static void CP_Ind_HL();
    static void ADC_A_v(BYTE v);
    static void ADC_R(BYTE &reg);
    static void SUB_A_v(BYTE v);
    static void SUB_R(BYTE &reg);
    static void SBC_A_v(BYTE v);
    static void SBC_R(BYTE &reg);
    static bool GetParity(BYTE b);
    static void SetFlagsAfterLogicalOp(BYTE b);
    static void SetFlagsAfterShiftOp(BYTE b);
    static void SetFlagsAfterBIT();
    static void AND_A_v(BYTE v);
    static void AND_R(BYTE &reg);
    static void XOR_A_v(BYTE v);
    static void XOR_R(BYTE &reg);
    static void OR_A_v(BYTE v);
    static void OR_R(BYTE &reg);
    static void CP_v(BYTE v);
    static void CP_R(BYTE &reg);
    static void RET(bool condition);
    static void JP(bool condition);
    static void CALL(bool condition);
    static void RST(BYTE address);
    static void RLC_R(BYTE &reg);
    static void RRC_R(BYTE &reg);
    static void RL_R(BYTE &reg);
    static void RR_R(BYTE &reg);
    static void SLA_R(BYTE &reg);
    static void SRA_R(BYTE &reg);
    static void SLL_R(BYTE &reg);
    static void SRL_R(BYTE &reg);
    static void BIT_x_R(int X, BYTE * R);
    static void RES_x_R(int X, BYTE * R);
    static void SET_x_R(int X, BYTE * R);
    static void RLC_IDX_R(BYTE &reg);
    static void RRC_IDX_R(BYTE &reg);
    static void RL_IDX_R(BYTE &reg);
    static void RR_IDX_R(BYTE &reg);
    static void SLA_IDX_R(BYTE &reg);
    static void SRA_IDX_R(BYTE &reg);
    static void SLL_IDX_R(BYTE &reg);
    static void SRL_IDX_R(BYTE &reg);
    static void BIT_x_IDX(int X);
    static void RES_x_IDX(int X, BYTE &reg);
    static void SET_x_IDX(int X, BYTE &reg);
    static void ADD_IDX_RR(Reg16 reg);
    static void SBC_HL_RR(Reg16 reg);
    static void ADC_HL_RR(Reg16 reg);
    static void NEG();
    static void LDI(bool R);
    static void CPI(bool R);
    static void LDD(bool R);
    static void CPD(bool R);
    static void RLCA();
    static void RRCA();
    static void LD_A_I_RR(Reg16 reg);
    static void EX_AF_AF_();
    static void EX_DE_HL();
    static void EXX();
    static void EX_HL_Ind_SP();
    static void EX_IDX_Ind_SP();
    static void JP_HL();
    static void JP_IDX();
    static void DJNZ();
    static void RLA();
    static void RRA();
    static void DAA();
    static void CPL();
    static void INC_Ind_HL();
    static void DEC_Ind_HL();
    static void LD_Ind_HL_n();
    static void SCF();
    static void CCF();
    static void LD_SP_HL();
    static void LD_SP_IDX();
    static void IN_R_PortBC(BYTE &reg);
    static void OUT_PortBC_R(BYTE &reg);
    static void LD_A_I();
    static void LD_I_A();
    static void LD_A_R();
    static void LD_R_A();
    static void RLD();
    static void RRD();
    static void IM(int mode);
    static void ShiftOpIndHL(BYTE opCode);
};

#endif // Z80_H

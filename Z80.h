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

struct Z80DebugState
{
    word AF, BC, DE, HL, IX, IY, PC, SP;
    word AF_, BC_, DE_, HL_;
    bool fS, fZ, f5, fH, f3, fP, fN, fC;
    bool InterruptRequest, IFF1, IFF2;
    BYTE R, I, im;
    dword nops;
};

class Z80
{
public:
    Z80();
    void Reset();
    void Clock();
    void Clock2();
    void IRQ();
    // Bus signals read/written by CPC orchestration and GateArray IRQ path
    bool WAIT;
    bool stopPoint;
    bool InterruptRequest;

    inline word GetPC() const { return PC; }
    inline word GetSP() const { return SP; }
    void ResetNopCounter() { nops = 0; }
    Z80DebugState GetDebugState();

private:
    BYTE tCycle;
    BYTE mCycle;
    bool MREQ, IORQ, RD, WR;
    word PC, SP;
    BYTE A, F, B, C, D, E, H, L, IXH, IXL, IYH, IYL;
    BYTE A_, F_, B_, C_, D_, E_, H_, L_;
    BYTE t16H, t16L, SPH, SPL;
    Reg16 AF, BC, DE, HL, IX, IY;
    Reg16 AF_, BC_, DE_, HL_;
    bool fS, fZ, fH, fP, fN, fC, f3, f5;
    BYTE I, R;
    BYTE IR;
    BYTE DR;
    word AR;
    IDMode idMode;
    MCycleType mCycleType;
    bool IFF1;
    bool IFF2;
    bool EIRequest;
    bool halted;
    BYTE im;
    dword nops;
    bool M1;

    void EncodeF();

    void RunMCycle();
    void RunTCycle();

    bool Step_basic();
    bool Step_misc();
    bool Step_CB();
    bool Step_IDX();
    bool Step_IDX_2();
    bool Step_IDX_3();
    bool Step_IDX_CB();
    bool Step_Int_Exec();
    bool ProcessINT();
    bool ProcessFETCH();
    bool ProcessREAD();
    bool ProcessREAD4();
    bool ProcessREAD5();
    bool ProcessWRITE();
    bool ProcessWRITE4();
    bool ProcessWRITE5();
    bool ProcessWRITEI();
    bool ProcessWRITED();
    bool ProcessIN();
    bool ProcessOUT();
    bool ProcessRELADDR();
    bool ProcessALU(BYTE length);
    void FinishInstruction();
    BYTE t8;
    Reg16 t16, tt16;
    sbyte index;
    BYTE tByte;
    BYTE opCode;
    Reg16 *IDX;
    BYTE t_cp;
    bool tC;
    int tCV;
    BYTE t;
    word w1, w2, w3;
    int i1, i2, i3;
    short s1;
    BYTE intACK;
    bool lastMCycle;
    bool lastTCycle;

    // 8 bit arithmetic and logic for A
    void NEG();
    void DAA();
    void CPL();
    void ADD_v(BYTE v);
    void ADC_v(BYTE v);
    void SUB_v(BYTE v);
    void SBC_v(BYTE v);
    void CP_v(BYTE v);
    void AND_v(BYTE v);
    void XOR_v(BYTE v);
    void OR_v(BYTE v);
    void INC_R(BYTE &reg);
    void DEC_R(BYTE &reg);
    bool ADD_n();
    bool ADC_n();
    bool SUB_n();
    bool SBC_n();
    bool AND_n();
    bool XOR_n();
    bool OR_n();
    bool CP_n();
    // 8 bit arithmetic and logic for (HL)
    bool ADD_Ind_HL();
    bool ADC_Ind_HL();
    bool SUB_Ind_HL();
    bool SBC_Ind_HL();
    bool CP_Ind_HL();
    bool AND_Ind_HL();
    bool XOR_Ind_HL();
    bool OR_Ind_HL();
    bool INC_Ind_HL();
    bool DEC_Ind_HL();
    // 8 bit arithmetic and logic for (IDX+d)
    bool ADD_Ind_IDX();
    bool ADC_Ind_IDX();
    bool SUB_Ind_IDX();
    bool SBC_Ind_IDX();
    bool CP_Ind_IDX();
    bool AND_Ind_IDX();
    bool XOR_Ind_IDX();
    bool OR_Ind_IDX();
    bool INC_Ind_IDX();
    bool DEC_Ind_IDX();
    // 16 bit arithmetic
    bool ADD_HL_vv(word w);
    bool ADC_HL_vv(word w);
    bool SBC_HL_vv(word w);
    bool ADD_IDX_vv(word w);
    bool INC_WW(word &w);
    bool DEC_WW(word &w);
    bool INC_RR(Reg16 reg);
    bool DEC_RR(Reg16 reg);
    bool ADD_IDX_RR(Reg16 reg);
    // Shift
    void RLCA();
    void RRCA();
    void RLA();
    void RRA();
    void RLC_R(BYTE &reg);
    void RRC_R(BYTE &reg);
    void RL_R(BYTE &reg);
    void RR_R(BYTE &reg);
    void SLA_R(BYTE &reg);
    void SRA_R(BYTE &reg);
    void SLL_R(BYTE &reg);
    void SRL_R(BYTE &reg);
    bool ShiftOpIndHL(BYTE opCode);
    void RLC_IDX_R(BYTE &reg);
    void RRC_IDX_R(BYTE &reg);
    void RL_IDX_R(BYTE &reg);
    void RR_IDX_R(BYTE &reg);
    void SLA_IDX_R(BYTE &reg);
    void SRA_IDX_R(BYTE &reg);
    void SLL_IDX_R(BYTE &reg);
    void SRL_IDX_R(BYTE &reg);
    bool RLD();
    bool RRD();
    // Bitwise op
    void BIT_x_R(int X, BYTE * R);
    void RES_x_R(int X, BYTE * R);
    void SET_x_R(int X, BYTE * R);
    bool BIT_x_Ind_HL(int X);
    bool RES_x_Ind_HL(int X);
    bool SET_x_Ind_HL(int X);
    void BIT_x_IDX(int X);
    void RES_x_IDX(int X, BYTE &reg);
    void SET_x_IDX(int X, BYTE &reg);
    // 8 bit load
    bool LD_Ind_A();
    bool LD_A_Ind();
    bool LD_R_Ind_HL(BYTE &reg);
    bool LD_Ind_HL_R(BYTE &reg);
    bool LD_R_n(BYTE &reg);
    bool LD_Ind_RR_A(Reg16 reg);
    bool LD_A_I();
    bool LD_I_A();
    bool LD_A_R();
    bool LD_R_A();
    // 16 bit load
    bool LD_RR_addr(Reg16 reg);
    bool LD_Ind_RR(Reg16 reg);
    bool LD_RR_Ind(Reg16 reg);
    bool LD_RR_nn(Reg16 reg);
    bool LD_SP_HL();
    bool LD_SP_IDX();
    bool LD_WW_nn(word &w);
    bool LD_Ind_WW(word w);
    bool LD_WW_Ind(word &w);
    bool LD_A_Ind_RR(Reg16 reg);
    bool LD_Ind_HL_n();
    bool PUSH_RR(Reg16 reg);
    bool POP_RR(Reg16 reg);
    // Jump
    bool JR(bool condition);
    bool JP(bool condition);
    bool CALL(bool condition);
    bool RST(BYTE address);
    bool RET();
    bool RET(bool condition);
    void JP_HL();
    void JP_IDX();
    bool DJNZ();
    // Block
    bool LDI(bool R);
    bool CPI(bool R);
    bool LDD(bool R);
    bool CPD(bool R);
    bool INI(bool R, bool dir);
    bool OUTI(bool R, bool dir);
    // Interchange
    void EX_AF_AF_();
    void EX_DE_HL();
    void EXX();
    bool EX_HL_Ind_SP();
    bool EX_IDX_Ind_SP();
    // I/O
    bool IN_R_PortBC(BYTE &reg);
    bool IN_A_n();
    bool OUT_PortBC_R(BYTE &reg);
    bool OUT_n_A();
    // Control
    void SCF();
    void CCF();
    void IM(int mode);
    // Helpers
    bool GetParity(BYTE b);

    void DecodeF();
    void SetFlagsAfterShiftOp(BYTE b);
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

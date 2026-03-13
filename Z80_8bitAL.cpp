#include "Z80.h"

#define FinishInstruction { mCycleType = MCycleType::FETCH; idMode = IDMode::BASIC; }

#define SetStandardFlags(r) \
fS = r & 0x80;\
    fZ = r == 0;\
    f3 = r & 0x08;\
    f5 = r & 0x20;

#define SetLogOpFlags(h) \
fH = h;\
    fP = GetParity(A);\
    fN = false;\
    fC = false;\
    SetStandardFlags(A)

#define ALO_n(op) \
    switch(mCycle)\
{\
    case 1:\
        mCycleType = MCycleType::READ;\
        AR = PC++;\
        break;\
    case 2:\
        op(DR);\
        FinishInstruction\
        break;\
}

#define ALO_Ind(op) \
switch(mCycle)\
    {\
        case 1:\
            mCycleType = MCycleType::READ;\
            AR = HL.Get();\
            break;\
        case 2:\
            op(DR);\
            FinishInstruction\
            break;\
    }

#define ALO_Ind_INC_DEC(op) \
switch(mCycle)\
    {\
        case 1:\
            mCycleType = MCycleType::READ;\
            AR = HL.Get();\
            break;\
        case 2:\
            op(DR);\
            mCycleType = MCycleType::WRITE;\
            break;\
        case 3:\
            FinishInstruction\
            break;\
    }

#define ALO_Ind_IDX(op) \
switch(mCycle)\
{\
case 1:\
    mCycleType = MCycleType::READ;\
    AR = PC++;\
    break;\
case 2:\
    AR = IDX->Get() + (sbyte)DR;\
    break;\
case 3:\
    mCycleType = MCycleType::ALU;\
    op(DR);\
    break;\
case 4:\
    FinishInstruction\
        break;\
}

#define ALO_Ind_IDX_INC_DEC(op) \
switch(mCycle)\
    {\
        case 1:\
            mCycleType = MCycleType::READ;\
            AR = PC++;\
            break;\
        case 2:\
            AR = IDX->Get() + (sbyte)DR;\
            break;\
        case 3:\
            mCycleType = MCycleType::WRITE;\
            op(DR);\
            break;\
        case 4:\
            mCycleType = MCycleType::ALU;\
            break;\
        case 5:\
            FinishInstruction\
            break;\
    }

void Z80::ADD_v(BYTE v)
{
    t_cp = A + v;
    fP = ((A ^ t_cp) & (v ^ t_cp)) & 0x80;
    fC = (A + v) > 0xFF;
    fH = ((A & 0xF) + (v & 0xF)) & 0x10;
    A = t_cp;
    fN = false;
    SetStandardFlags(A)
}

void Z80::ADC_v(BYTE v)
{
    s1 = A + v + (fC ? 1 : 0);
    fH = ((A & 0xF) + (v & 0xF) + (fC ? 1 : 0)) & 0x10;
    fP = ((A ^ s1) & (v ^ s1)) & 0x80;
    fC = s1 > 0xFF;
    A = s1 & 0xFF;
    fN = false;
    SetStandardFlags(A)
}

void Z80::SUB_v(BYTE v)
{
    t_cp = A - v;
    fP = ((A ^ v) & (A ^ t_cp)) & 0x80;
    fH = (A & 0xF) < (v & 0xF);
    fC = A < v;
    A = t_cp;
    fN = true;
    SetStandardFlags(A)
}

void Z80::SBC_v(BYTE v)
{
    s1 = A - v - (fC ? 1 : 0);
    fP = ((A ^ v) & (A ^ s1)) & 0x80;
    fH = (A & 0xF) < ((v & 0xF) + (fC ? 1 : 0));
    fC = s1 < 0;
    A = s1 & 0xFF;
    fN = true;
    SetStandardFlags(A)
}

void Z80::CP_v(BYTE v)
{
    t_cp = A - v;
    fP = ((A ^ v) & (A ^ t_cp)) & 0x80;
    fH = (A & 0xF) < (v & 0xF);
    fC = A < v;
    fN = true;
    fS = t_cp & 0x80;
    fZ = t_cp == 0;
    f3 = v & 0x08;
    f5 = v & 0x20;
}

void Z80::AND_v(BYTE v)
{
    A &= v;
    SetLogOpFlags(true)
}

void Z80::XOR_v(BYTE v)
{
    A ^= v;
    SetLogOpFlags(false)
}

void Z80::OR_v(BYTE v)
{
    A |= v;
    SetLogOpFlags(false)
}

void Z80::ADD_n()
{
    ALO_n(ADD_v);
    intAlign = true;
}

void Z80::ADC_n()
{
    ALO_n(ADC_v);
    intAlign = true;
}

void Z80::SUB_n()
{
    ALO_n(SUB_v);
    intAlign = true;
}

void Z80::SBC_n()
{
    ALO_n(SBC_v);
    intAlign = true;
}

void Z80::CP_n()
{
    ALO_n(CP_v);
    intAlign = true;
}

void Z80::AND_n()
{
    ALO_n(AND_v);
    intAlign = true;
}

void Z80::XOR_n()
{
    ALO_n(XOR_v);
    intAlign = true;
}

void Z80::OR_n()
{
    ALO_n(OR_v);
    intAlign = true;
}

void Z80::INC_R(BYTE &reg)
{
    fH = (reg & 0xF) == 0xF;
    fP = reg == 0x7F;
    reg++;
    fN = false;
    SetStandardFlags(reg)
}

void Z80::DEC_R(BYTE &reg)
{
    fH = (reg & 0xF) == 0x0;
    fP = reg == 0x80;
    reg--;
    fN = true;
    SetStandardFlags(reg)
}

void Z80::ADD_Ind_HL()
{
    ALO_Ind(ADD_v);
    intAlign = true;
}

void Z80::ADC_Ind_HL()
{
    ALO_Ind(ADC_v);
    intAlign = true;
}

void Z80::SUB_Ind_HL()
{
    ALO_Ind(SUB_v);
    intAlign = true;
}

void Z80::SBC_Ind_HL()
{
    ALO_Ind(SBC_v);
    intAlign = true;
}

void Z80::CP_Ind_HL()
{
    ALO_Ind(CP_v);
    intAlign = true;
}

void Z80::AND_Ind_HL()
{
    ALO_Ind(AND_v);
    intAlign = true;
}

void Z80::XOR_Ind_HL()
{
    ALO_Ind(XOR_v);
    intAlign = true;
}

void Z80::OR_Ind_HL()
{
    ALO_Ind(OR_v);
    intAlign = true;
}

void Z80::INC_Ind_HL()
{
    ALO_Ind_INC_DEC(INC_R);
    intAlign = true;
}

void Z80::DEC_Ind_HL()
{
    ALO_Ind_INC_DEC(DEC_R);
    intAlign = true;
}

void Z80::ADD_Ind_IDX()
{
    ALO_Ind_IDX(ADD_v);
    intAlign = true;
}

void Z80::ADC_Ind_IDX()
{
    ALO_Ind_IDX(ADC_v);
    intAlign = true;
}

void Z80::SUB_Ind_IDX()
{
    ALO_Ind_IDX(SUB_v);
    intAlign = true;
}

void Z80::SBC_Ind_IDX()
{
    ALO_Ind_IDX(SBC_v);
    intAlign = true;
}

void Z80::CP_Ind_IDX()
{
    ALO_Ind_IDX(CP_v);
    intAlign = true;
}

void Z80::AND_Ind_IDX()
{
    ALO_Ind_IDX(AND_v);
    intAlign = true;
}

void Z80::XOR_Ind_IDX()
{
    ALO_Ind_IDX(XOR_v);
    intAlign = true;
}

void Z80::OR_Ind_IDX()
{
    ALO_Ind_IDX(OR_v);
    intAlign = true;
}

void Z80::INC_Ind_IDX()
{
    ALO_Ind_IDX_INC_DEC(INC_R);
    intAlign = true;
}

void Z80::DEC_Ind_IDX()
{
    ALO_Ind_IDX_INC_DEC(DEC_R);
    intAlign = true;
}

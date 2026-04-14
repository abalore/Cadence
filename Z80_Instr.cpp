#include "Z80.h"

bool Z80::GetParity(BYTE b)
{
    b ^= b >> 4;
    b ^= b >> 2;
    b ^= b >> 1;
    return !(b & 0x01);
}

void Z80::BIT_x_R(int X, BYTE * R)
{
    fZ = !(*R & (1 << X));
    fH = true;
    fN = false;
    fS = X == 7 ? *R & 0x80 : 0;
    fP = fZ;
}

void Z80::RES_x_R(int X, BYTE * R)
{
    *R &= (BYTE)((1 << X) ^ 0xFF);
}

void Z80::SET_x_R(int X, BYTE * R)
{
    *R |= (BYTE)(1 << X);
}

void Z80::RLC_IDX_R(BYTE &reg)
{
    RLC_R(tByte);
    reg = tByte;
}

void Z80::RRC_IDX_R(BYTE &reg)
{
    RRC_R(tByte);
    reg = tByte;
}

void Z80::RL_IDX_R(BYTE &reg)
{
    RL_R(tByte);
    reg = tByte;
}

void Z80::RR_IDX_R(BYTE &reg)
{
    RR_R(tByte);
    reg = tByte;
}

void Z80::SLA_IDX_R(BYTE &reg)
{
    SLA_R(tByte);
    reg = tByte;
}

void Z80::SRA_IDX_R(BYTE &reg)
{
    SRA_R(tByte);
    reg = tByte;

}

void Z80::SLL_IDX_R(BYTE &reg)
{
    SLL_R(tByte);
    reg = tByte;

}

void Z80::SRL_IDX_R(BYTE &reg)
{
    SRL_R(tByte);
    reg = tByte;
}

void Z80::BIT_x_IDX(int X)
{
    BIT_x_R(X, &tByte);
    f3 = AR & 0x0800;
    f5 = AR & 0x2000;
}

void Z80::RES_x_IDX(int X, BYTE &reg)
{
    tByte &= (BYTE)((1 << X) ^ 0xFF);
    reg = tByte;
}

void Z80::SET_x_IDX(int X, BYTE &reg)
{
    tByte |= (BYTE)(1 << X);
    reg = tByte;
}

bool Z80::ADD_IDX_RR(Reg16 reg)
{
    return ADD_IDX_vv(reg.Get());
}

bool Z80::ADD_IDX_vv(word w)
{
    switch(mCycle)
    {
    case 1:
        w1 = (*IDX).Get();
        Z80::mCycleType = MCycleType::ALU4;
        break;
    case 2:
        fC = w1 + w > 0xFFFF;
        fH = (w1 & 0xFFF) + (w & 0xFFF) > 0xFFF;
        (*IDX).Set(w1 + w);
        Z80::mCycleType = MCycleType::ALU3;
        break;
    case 3:
        fN = false;
        f3 = IXH & 0x08;
        f5 = IXH & 0x20;
        return true;
    }
    return false;
}

void Z80::NEG()
{
    fZ = A == 0;
    fC = A != 0x00;
    fP = A == 0x80;
    fH = A & 0xF;
    A = -A;
    fS = (A & 0x80) > 0;
    fN = true;
    f3 = A & 0x08;
    f5 = A & 0x20;
}

bool Z80::LDI(bool R)
{
    switch(mCycle)
    {
    case 1:
        w1 = BC.Get() - 1;
        w2 = HL.Get();
        w3 = DE.Get();
        mCycleType = MCycleType::READ;
        AR = w2;
        break;
    case 2:
        t8 = DR + A;
        f3 = t8 & 0x08;
        f5 = t8 & 0x02;
        mCycleType = MCycleType::WRITEI;
        AR = w3;
        break;
    case 3:
        fP = w1 != 0;
        fH = false;
        fN = false;
        if (!R || w1 == 0)
            return true;
        mCycleType = MCycleType::ALU5;
        break;
    case 4:
        PC -= 2;
        return true;
    }
    return false;
}

bool Z80::CPI(bool R)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = HL.Get();
        break;
    case 2:
        mCycleType = MCycleType::ALU5;
        HL.Set(HL.Get() + 1);
        BC.Set(BC.Get() - 1);
        break;
    case 3:
        fH = (A & 0x0F) < (DR & 0x0F);
        fS = (A - DR) & 0x80;
        fZ = A == DR;
        fP = BC.Get() != 0;
        fN = true;
        t8 = (A - DR) + (fH ? 1 : 0);
        f3 = t8 & 0x08;
        f5 = t8 & 0x02;
        if (!R || BC.Get() == 0 || A == DR)
            return true;
        mCycleType = MCycleType::ALU5;
        break;
    case 4:
        PC -= 2;
        return true;
    }
    return false;
}

bool Z80::LDD(bool R)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = HL.Get();
        break;
    case 2:
        t8 = DR + A;
        f3 = t8 & 0x08;
        f5 = t8 & 0x02;
        mCycleType = MCycleType::WRITED;
        AR = DE.Get();
        break;
    case 3:
        fP = BC.Get() != 0;
        fH = false;
        fN = false;
        if (!R || BC.Get() == 0)
            return true;
        mCycleType = MCycleType::ALU5;
        break;
    case 4:
        PC -= 2;
        return true;
    }
    return false;
}

bool Z80::CPD(bool R)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = HL.Get();
        break;
    case 2:
        mCycleType = MCycleType::ALU5;
        HL.Set(HL.Get()-1);
        BC.Set(BC.Get()-1);
        break;
    case 3:
        fH = (A & 0xF) < (DR & 0xF);
        fS = ((A - DR) & 0x80) > 0;
        fZ = A == DR;
        t8 = (A - DR) + (fH ? 1 : 0);
        f3 = t8 & 0x08;
        f5 = t8 & 0x02;
        fP = BC.Get() != 0;
        fN = true;
        if (!R || BC.Get() == 0 || A == DR)
            return true;
        mCycleType = MCycleType::ALU5;
        break;
    case 4:
        PC -= 2;
        return true;
    }
    return false;
}

void Z80::RLCA()
{
    fC = (A & 0x80) > 0;
    A <<= 1;
    if (fC) A |= 0x01;
    fH = false;
    fN = false;
    f3 = A & 0x08;
    f5 = A & 0x20;
}

void Z80::RRCA()
{
    fC = (A & 0x01) > 0;
    A >>= 1;
    if (fC) A |= 0x80;
    fH = false;
    fN = false;
    f3 = A & 0x08;
    f5 = A & 0x20;
}

bool Z80::LD_A_Ind_RR(Reg16 reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = reg.Get();
        break;
    case 2:
        A = DR;
        return true;
    }
    return false;
}

void Z80::EncodeF()
{
    F = (fS << 7)
        + (fZ << 6)
        + (f5 << 5)
        + (fH << 4)
        + (f3 << 3)
        + (fP << 2)
        + (fN << 1)
        + fC;
}

void Z80::DecodeF()
{
    fS = F & 0x80;
    fZ = F & 0x40;
    f5 = F & 0x20;
    fH = F & 0x10;
    f3 = F & 0x08;
    fP = F & 0x04;
    fN = F & 0x02;
    fC = F & 0x01;
}

void Z80::EX_AF_AF_()
{
    t = A; A = A_; A_ = t;
    EncodeF();
    t = F; F = F_; F_ = t;
    DecodeF();
}

void Z80::EX_DE_HL()
{
    t = D; D = H; H = t;
    t = E; E = L; L = t;
}

void Z80::EXX()
{
    t = B; B = B_; B_ = t;
    t = C; C = C_; C_ = t;
    t = D; D = D_; D_ = t;
    t = E; E = E_; E_ = t;
    t = H; H = H_; H_ = t;
    t = L; L = L_; L_ = t;
}

bool Z80::EX_HL_Ind_SP()
{
    switch(mCycle)
    {
    case 1: // 4
        mCycleType = MCycleType::READ;
        AR = SP;
        break;
    case 2: // 3
        *t16.L = DR;
        AR++;
        mCycleType = MCycleType::READ4;
        break;
    case 3: // 4
        *t16.H = DR;
        DR = H;
        mCycleType = MCycleType::WRITE;
        break;
    case 4: // 3
        AR--;
        DR = L;
        mCycleType = MCycleType::WRITE5;
        break;
    case 5: // 5
        HL.Set(t16.Get());
        return true;
    }
    return false;
}

bool Z80::EX_IDX_Ind_SP()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = SP;
        break;
    case 2:
        mCycleType = MCycleType::READ4;
        *t16.L = DR;
        AR++;
        break;
    case 3:
        *t16.H = DR;
        DR = *(*IDX).H;
        mCycleType = MCycleType::WRITE;
        break;
    case 4:
        AR--;
        DR = *(*IDX).L;
        mCycleType = MCycleType::WRITE5;
        break;
    case 5:
        (*IDX).Set(t16.Get());
        return true;
    }
    return false;
}

void Z80::JP_HL()
{
    PC = HL.Get();
}

void Z80::JP_IDX()
{
    PC = (*IDX).Get();
}

void Z80::RLA()
{
    tC = A & 0x80;
    A <<= 1;
    if (fC) A |= 0x01;
    fC = tC;
    fH = false;
    fN = false;
    f3 = A & 0x08;
    f5 = A & 0x20;
}

void Z80::RRA()
{
    tC = A & 0x01;
    A >>= 1;
    if (fC) A |= 0x80;
    fC = tC;
    fH = false;
    fN = false;
    f3 = A & 0x08;
    f5 = A & 0x20;
}

void Z80::DAA()
{
    t8 = 0;
    tC = fC;
    t = A & 0x0F;
    tByte = A >> 4;
    if (fH || t > 9)
        t8 |= 0x06;
    if (fC || tByte > 9 || (tByte == 9 && t > 9)) {
        t8 |= 0x60;
        tC = true;
    }
    if (fN) {
        fH = (fH && (t < 6));
        A -= t8;
    } else {
        fH = (t > 9);
        A += t8;
    }
    fS = (A & 0x80) != 0;
    fZ = (A == 0x00);
    fP = GetParity(A);
    fC = tC;
    f3 = A & 0x08;
    f5 = A & 0x20;
}

void Z80::CPL()
{
    A ^= 0xFF;
    fH = true;
    fN = true;
    f3 = A & 0x08;
    f5 = A & 0x20;
}

bool Z80::LD_Ind_HL_n()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = PC++;
        break;
    case 2:
        AR = HL.Get();
        mCycleType = MCycleType::WRITE;
        break;
    case 3:
        return true;
    }
    return false;
}

void Z80::SCF()
{
    fC = true;
    fN = false;
    fH = false;
    f3 = A & 0x08;
    f5 = A & 0x20;
}

void Z80::CCF()
{
    fH = fC;
    fC = !fC;
    fN = false;
    f3 = A & 0x08;
    f5 = A & 0x20;
}

bool Z80::LD_SP_HL()
{
    switch(mCycle)
    {
    case 1:
        SP = HL.Get();
        return true;
    }
    return false;
}

bool Z80::LD_SP_IDX()
{
    switch(mCycle)
    {
    case 1:
        SP = (*IDX).Get();
        return true;
    }
    return false;
}

bool Z80::IN_R_PortBC(BYTE &reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::IN;
        AR = BC.Get();
        break;
    case 2:
        reg = DR;
        fS = DR & 0x80;
        fZ = DR == 0;
        fH = false;
        fP = GetParity(DR);
        fN = false;
        f3 = reg & 0x08;
        f5 = reg & 0x20;
        return true;
    }
    return false;
}

bool Z80::IN_A_n()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = PC++;
        break;
    case 2:
        mCycleType = MCycleType::IN;
        AR = A * 256 + DR;
        break;
    case 3:
        A = DR;
        fS = DR & 0x80;
        fZ = DR == 0;
        fH = false;
        fP = GetParity(DR);
        fN = false;
        f3 = A & 0x08;
        f5 = A & 0x20;
        return true;
    }
    return false;
}

bool Z80::OUT_PortBC_R(BYTE &reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::OUT;
        AR = BC.Get();
        DR = reg;
        break;
    case 2:
        return true;
    }
    return false;
}

bool Z80::OUT_n_A()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = PC++;
        break;
    case 2:
        mCycleType = MCycleType::OUT;
        AR = DR;
        DR = A;
        break;
    case 3:
        return true;
    }
    return false;
}

bool Z80::LD_A_I()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::ALU5;
        A = I;
        break;
    case 2:
        fZ = A == 0;
        fS = A & 0x80;
        fH = false;
        fN = false;
        fP = IFF1;
        f3 = A & 0x08;
        f5 = A & 0x20;
        return true;
    }
    return false;
}

bool Z80::LD_I_A()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::ALU5;
        I = A;
        break;
    case 2:
        return true;
    }
    return false;
}

bool Z80::LD_A_R()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::ALU5;
        A = R;
        break;
    case 2:
        fP = IFF2;
        fZ = A == 0;
        fS = A & 0x80;
        fH = false;
        fN = false;
        f3 = A & 0x08;
        f5 = A & 0x20;
        return true;
    }
    return false;
}

bool Z80::LD_R_A()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::ALU5;
        R = A;
        break;
    case 2:
        return true;
    }
    return false;
}

bool Z80::RLD()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = HL.Get();
        break;
    case 2:
        t8 = DR;
        DR = (BYTE)((DR << 4) + (A & 0x0F));
        mCycleType = MCycleType::WRITE4;
        break;
    case 3:
        A = (BYTE)((A & 0xF0) + (t8 >> 4));
        mCycleType = MCycleType::WRITE;
        break;
    case 4:
        fP = GetParity(A);
        fZ = A == 0;
        fS = (A & 0x80) > 0;
        fH = false;
        fN = false;
        f3 = A & 0x08;
        f5 = A & 0x20;
        return true;
    }
    return false;
}

bool Z80::RRD()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        AR = HL.Get();
        break;
    case 2:
        t8 = DR;
        DR = (BYTE)((DR >> 4) + (A << 4));
        mCycleType = MCycleType::WRITE4;
        break;
    case 3:
        A = (BYTE)((A & 0xF0) + (t8 & 0x0F));
        mCycleType = MCycleType::WRITE;
        break;
    case 5:
        fP = GetParity(A);
        fZ = A == 0;
        fS = (A & 0x80) > 0;
        fH = false;
        fN = false;
        f3 = A & 0x08;
        f5 = A & 0x20;
        return true;
    }
    return false;
}

void Z80::IM(int mode)
{
    im = mode;
}

bool Z80::INI(bool R, bool dir)
{
    switch(mCycle)
    {
    case 1:  // 5T
        mCycleType = MCycleType::IN;
        AR = BC.Get();
        break;
    case 2: // 4T
        if (dir)
            t8 = DR + ((C + 1) & 0xFF);
        else
            t8 = DR + ((C - 1) & 0xFF);
        f3 = t8 & 0x08;
        f5 = t8 & 0x20;
        mCycleType = MCycleType::WRITE;
        AR = HL.Get();
        break;
    case 3: // 3T
        if (dir)
            HL.Set(HL.Get() + 1);
        else
            HL.Set(HL.Get() - 1);
        B--;
        fS = (DR & 0x80) > 0;
        fZ = B == 0;
        fN = true;
        if (!R || B == 0)
            return true;
        mCycleType = MCycleType::ALU5;
        break;
    case 5:
        PC -= 2;
        return true;
    }
    return false;
}

bool Z80::OUTI(bool R, bool dir)
{
    switch(mCycle)
    {
    case 1: // 5
        mCycleType = MCycleType::READ;
        AR = HL.Get();
        break;
    case 2: // 3
        B--;
        AR = BC.Get();
        mCycleType = MCycleType::OUT;
        break;
    case 3: // 4
        if (dir)
            HL.Set(HL.Get() + 1);
        else
            HL.Set(HL.Get() - 1);
        fS = (DR & 0x80) > 0;
        fZ = B == 0;
        fN = true;
        if (!R || B == 0)
            return true;
        mCycleType = MCycleType::ALU4;
        break;
    case 4:
        PC -= 2;
        return true;
    }
    return false;
}



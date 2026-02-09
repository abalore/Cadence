#include "Headers/Z80.h"

void Z80::INC_R(BYTE &reg)
{
    fH = (reg & 0xF) == 0xF;
    fP = reg == 0x7F;
    reg++;
    fS = reg & 0x80;
    fZ = reg == 0;
    fN = false;
}

void Z80::DEC_R(BYTE &reg)
{
    fH = (reg & 0xF) == 0x0;
    fP = reg == 0x80;
    reg--;
    fS = reg & 0x80;
    fZ = reg == 0;
    fN = true;
}

void Z80::JR(bool condition)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = PC;
        break;
    case 2:
        tAddr++;
        PC = tAddr;
        if (!condition)
            FinishInstruction();
        else
            mCycleType = MCycleType::ALU;
        break;
    case 3:
        PC += (sbyte)DR;
        FinishInstruction();
        break;
    }
}

void Z80::LD_Ind_RR(Reg16 reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = PC;
        break;
    case 2:
        *t16.L = DR;
        tAddr++;
        break;
    case 3:
        *t16.H = DR;
        tAddr++;
        PC = tAddr;
        tAddr = t16.Get();
        mCycleType = MCycleType::WRITE;
        DR = *reg.L;
        break;
    case 4:
        tAddr++;
        DR = *reg.H;
        break;
    case 5:
        FinishInstruction();
        break;
    }
}

void Z80::LD_RR_Ind(Reg16 reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = PC;
        break;
    case 2:
        *t16.L = DR;
        tAddr++;
        break;
    case 3:
        *t16.H = DR;
        tAddr++;
        PC = tAddr;
        tAddr = t16.Get();
        break;
    case 4:
        *reg.L = DR;
        tAddr++;
        break;
    case 5:
        *reg.H = DR;
        FinishInstruction();
        break;
    }
}

void Z80::LD_Ind_A()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = PC;
        break;
    case 2:
        *t16.L = DR;
        tAddr++;
        break;
    case 3:
        *t16.H = DR;
        tAddr++;
        PC = tAddr;
        tAddr = t16.Get();
        mCycleType = MCycleType::WRITE;
        DR = A;
        break;
    case 4:
        FinishInstruction();
        break;
    }
}

void Z80::LD_A_Ind()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = PC;
        break;
    case 2:
        *t16.L = DR;
        tAddr++;
        break;
    case 3:
        *t16.H = DR;
        tAddr++;
        PC = tAddr;
        tAddr = t16.Get();
        break;
    case 4:
        A = DR;
        FinishInstruction();
        break;

    }
}

void Z80::LD_R_Ind_HL(BYTE &reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = HL.Get();
        break;
    case 2:
        reg = DR;
        FinishInstruction();
        break;
    }
}

void Z80::LD_Ind_HL_R(BYTE &reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::WRITE;
        tAddr = HL.Get();
        DR = reg;
        break;
    case 2:
        FinishInstruction();
        break;
    }
}

void Z80::LD_RR_nn(Reg16 reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = PC;
        break;
    case 2:
        *reg.L = DR;
        tAddr++;
        break;
    case 3:
        *reg.H = DR;
        tAddr++;
        PC = tAddr;
        FinishInstruction();
        break;
    }
}

void Z80::INC_RR(Reg16 reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::ALU;
        *reg.L = *reg.L + 1;
        break;
    case 2:
        if (*reg.L == 0)
            *reg.H = *reg.H + 1;
        FinishInstruction();
        break;
    }
}

void Z80::DEC_RR(Reg16 reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::ALU;
        *reg.L = *reg.L - 1;;
        break;
    case 2:
        if (*reg.L == 0xff)
            *reg.H = *reg.H - 1;
        FinishInstruction();
        break;
    }
}

void Z80::LD_R_n(BYTE &reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = PC;
        break;
    case 2:
        reg = DR;
        tAddr++;
        PC = tAddr;
        FinishInstruction();
        break;
    }
}

void Z80::LD_Ind_RR_A(Reg16 reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::WRITE;
        tAddr = reg.Get();
        DR = A;
        break;
    case 2:
        FinishInstruction();
        break;
    }
}

void Z80::PUSH_RR(Reg16 reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::WRITE;
        tAddr = SP.Get();
        tAddr--;
        DR = *reg.H;
        break;
    case 2:
        tAddr--;
        DR = *reg.L;
        break;
    case 3:
        mCycleType = MCycleType::ALU;
        SP.Set(tAddr);
        break;
    case 4:
        FinishInstruction();
        break;
    }
}

void Z80::POP_RR(Reg16 reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = SP.Get();
        break;
    case 2:
        *t16.L = DR;
        tAddr++;
        break;
    case 3:
        *t16.H = DR;
        tAddr++;
        SP.Set(tAddr);
        reg.Set(t16.Get());
        FinishInstruction();
        break;
    }
}

void Z80::ADD_HL_RR(Reg16 reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::ALU;
        fC = HL.Get() + reg.Get() > 0xFFFF;
        fP = HL.Get() + reg.Get() > 0xFFFF;
        fH = (HL.Get() & 0xFFF) + (reg.Get() & 0xFFF) > 0xFFF;
        break;
    case 2:
        H += *reg.H;
        if ((L + *reg.L) > 255) H++;
        L += *reg.L;
        break;
    case 3:
        fN = false;
        FinishInstruction();
        break;
    }
}

void Z80::ADD_A_v(BYTE v)
{
    int signedResult = (sbyte)A + (sbyte)v;
    fP = (signedResult > 127) || (signedResult < -128);
    fC = A + v > 255;
    fH = ((A & 0xF) + (v & 0xF)) > 0xF;
    A += v;
    fS = A & 0x80;
    fZ = A == 0;
    fN = false;
}

void Z80::ADD_n()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = PC;
        break;
    case 2:
        tAddr++;
        PC = tAddr;
        ADD_A_v(DR);
        FinishInstruction();
        break;
    }
}

void Z80::ADC_n()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = PC;
        break;
    case 2:
        tAddr++;
        PC = tAddr;
        ADC_A_v(DR);
        FinishInstruction();
        break;
    }
}

void Z80::SUB_n()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = PC;
        break;
    case 2:
        tAddr++;
        PC = tAddr;
        SUB_A_v(DR);
        FinishInstruction();
        break;
    }
}

void Z80::SBC_n()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = PC;
        break;
    case 2:
        tAddr++;
        PC = tAddr;
        SBC_A_v(DR);
        FinishInstruction();
        break;
    }
}

void Z80::AND_n()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = PC;
        break;
    case 2:
        tAddr++;
        PC = tAddr;
        AND_A_v(DR);
        FinishInstruction();
        break;
    }
}

void Z80::XOR_n()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = PC;
        break;
    case 2:
        tAddr++;
        PC = tAddr;
        XOR_A_v(DR);
        FinishInstruction();
        break;
    }
}

void Z80::OR_n()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = PC;
        break;
    case 2:
        tAddr++;
        PC = tAddr;
        OR_A_v(DR);
        FinishInstruction();
        break;
    }
}

void Z80::CP_n()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = PC;
        break;
    case 2:
        tAddr++;
        PC = tAddr;
        CP_v(DR);
        FinishInstruction();
        break;
    }
}

void Z80::ADD_R(BYTE &reg)
{
  ADD_A_v(reg);
}

void Z80::ADD_Ind_HL()
{
  switch(mCycle)
  {
    case 1:
      mCycleType = MCycleType::READ;
      tAddr = HL.Get();
      break;
    case 2:
      ADD_R(DR);
      FinishInstruction();
      break;
  }
}

void Z80::ADC_Ind_HL()
{
  switch(mCycle)
  {
    case 1:
      mCycleType = MCycleType::READ;
      tAddr = HL.Get();
      break;
    case 2:
      ADC_R(DR);
      FinishInstruction();
      break;
  }
}

void Z80::SUB_Ind_HL()
{
  switch(mCycle)
  {
    case 1:
      mCycleType = MCycleType::READ;
      tAddr = HL.Get();
      break;
    case 2:
      SUB_R(DR);
      FinishInstruction();
      break;
  }
}

void Z80::SBC_Ind_HL()
{
  switch(mCycle)
  {
    case 1:
      mCycleType = MCycleType::READ;
      tAddr = HL.Get();
      break;
    case 2:
      SBC_R(DR);
      FinishInstruction();
      break;
  }
}

void Z80::AND_Ind_HL()
{
  switch(mCycle)
  {
    case 1:
      mCycleType = MCycleType::READ;
      tAddr = HL.Get();
      break;
    case 2:
      AND_R(DR);
      FinishInstruction();
      break;
  }
}

void Z80::XOR_Ind_HL()
{
  switch(mCycle)
  {
    case 1:
      mCycleType = MCycleType::READ;
      tAddr = HL.Get();
      break;
    case 2:
      XOR_R(DR);
      FinishInstruction();
      break;
  }
}

void Z80::OR_Ind_HL()
{
  switch(mCycle)
  {
    case 1:
      mCycleType = MCycleType::READ;
      tAddr = HL.Get();
      break;
    case 2:
      OR_R(DR);
      FinishInstruction();
      break;
  }
}

void Z80::CP_Ind_HL()
{
  switch(mCycle)
  {
    case 1:
      mCycleType = MCycleType::READ;
      tAddr = HL.Get();
      break;
    case 2:
      CP_R(DR);
      FinishInstruction();
      break;
  }
}


void Z80::ADC_A_v(BYTE v)
{
    tCV = fC ? 1 : 0;
    int signedResult = (sbyte)A + (sbyte)v + tCV;
    fP = (signedResult > 127) || (signedResult < -128);
    fC = (A + v + tCV) > 255;
    fH = ((A & 0xF) + (v & 0xF) + tCV) > 0xF;
    A += v + tCV;
    fS = A & 0x80;
    fZ = A == 0;
    fN = false;
}

void Z80::ADC_R(BYTE &reg)
{
    ADC_A_v(reg);
}

void Z80::SUB_A_v(BYTE v)
{
  int signedResult = (sbyte)A - (sbyte)v;
  fP = (signedResult > 127) || (signedResult < -128);
  fH = (A & 0xF) < (v & 0xF);
  fC = A < v;
  A -= v;
  fS = (A & 0x80) > 0;
  fZ = A == 0;
  fN = true;
}

void Z80::SUB_R(BYTE &reg)
{
  SUB_A_v(reg);
}

void Z80::SBC_A_v(BYTE v)
{
    tCV = fC ? 1 : 0;
    int signedResult = (sbyte)A - (sbyte)v - tCV;
    fP = (signedResult > 127) || (signedResult < -128);
    fH = (A & 0xF) < ((v & 0xF) + tCV);
    fC = A < (v + tCV);
    A -= v + tCV;
    fS = (A & 0x80) > 0;
    fZ = A == 0;
    fN = true;
}

void Z80::SBC_R(BYTE &reg)
{
  SBC_A_v(reg);
}

bool Z80::GetParity(BYTE b)
{
  b ^= b >> 4;
  b ^= b >> 2;
  b ^= b >> 1;
  return (b & 0x01) == 0;
}

void Z80::SetFlagsAfterLogicalOp(BYTE b)
{
    fP = GetParity(b);
    fS = (b & 0x80) > 0;
    fZ = b == 0;
    fN = false;
    fC = false;
}

void Z80::SetFlagsAfterShiftOp(BYTE b)
{
    fP = GetParity(b);
    fS = (b & 0x80) > 0;
    fZ = b == 0;
    fH = false;
    fN = false;
}

void Z80::AND_A_v(BYTE v)
{
    A &= v;
    fH = true;
    SetFlagsAfterLogicalOp(A);
}

void Z80::AND_R(BYTE &reg)
{
  AND_A_v(reg);
}

void Z80::XOR_A_v(BYTE v)
{
  A ^= v;
  fH = false;
  SetFlagsAfterLogicalOp(A);
}

void Z80::XOR_R(BYTE &reg)
{
  XOR_A_v(reg);
}

void Z80::OR_A_v(BYTE v)
{
  A |= v;
  fH = false;
  SetFlagsAfterLogicalOp(A);
}

void Z80::OR_R(BYTE &reg)
{
  OR_A_v(reg);
}

void Z80::CP_v(BYTE v)
{
  t_cp = A;
  SUB_A_v(v);
  A = t_cp;
}

void Z80::CP_R(BYTE &reg)
{
  CP_v(reg);
}

void Z80::RET(bool condition)
{
  switch(mCycle)
  {
    case 1:
      mCycleType = MCycleType::READ;
      tAddr = SP.Get();
      break;
    case 2:
      *t16.L = DR;
      tAddr++;
      if (!condition) FinishInstruction();
      break;
    case 3:
      *t16.H = DR;
      tAddr++;
      SP.Set(tAddr);
      PC = t16.Get();
      FinishInstruction();
      break;
  }
}

void Z80::JP(bool condition)
{
  switch(mCycle)
  {
    case 1:
      mCycleType = MCycleType::READ;
      tAddr = PC;
      break;
    case 2:
      *t16.L = DR;
      tAddr++;
      break;
    case 3:
      *t16.H = DR;
      tAddr++;
      if (condition)
        PC = t16.Get();
      else
        PC = tAddr;
      FinishInstruction();
      break;
  }
}

void Z80::CALL(bool condition)
{
  switch(mCycle)
  {
    case 1:
      mCycleType = MCycleType::READ;
      tAddr = PC;
      break;
    case 2:
      *t16.L = DR;
      tAddr++;
      break;
    case 3:
      *t16.H = DR;
      tAddr++;
      PC = tAddr;
      if (condition)
      {
        mCycleType = MCycleType::WRITE;
        tAddr = SP.Get();
        tAddr--;
        DR = (BYTE)(PC >> 8);
      }
      else
      {
        FinishInstruction();
      }
      break;
    case 4:
      tAddr--;
      DR = (BYTE)(PC & 0xFF);
      break;
    case 5:
      SP.Set(tAddr);
      PC = t16.Get();
      FinishInstruction();
      break;
  }
}

void Z80::RST(BYTE address)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::WRITE;
        tAddr = SP.Get();
        tAddr--;
        DR = (BYTE)(PC >> 8);
        break;
    case 2:
        tAddr--;
        DR = (BYTE)(PC & 0xFF);
        break;
    case 3:
        mCycleType = MCycleType::ALU;
        SP.Set(tAddr);
        break;
    case 4:
        PC = address;
        break;
    case 5:
        FinishInstruction();
        break;
    }
}


void Z80::RLC_R(BYTE &reg)
{
    fC = (reg & 0x80) > 0;
    reg <<= 1;
    if (fC) reg++;
    SetFlagsAfterShiftOp(reg);
}

void Z80::RRC_R(BYTE &reg)
{
    fC = (reg & 0x01) > 0;
    reg >>= 1;
    if (fC) reg += 0x80;
    SetFlagsAfterShiftOp(reg);
}

void Z80::RL_R(BYTE &reg)
{
    tC = fC;
    fC = (reg & 0x80) > 0;
    reg <<= 1;
    if (tC) reg++;
    SetFlagsAfterShiftOp(reg);
}

void Z80::RR_R(BYTE &reg)
{
    tC = fC;
    fC = (reg & 0x01) > 0;
    reg >>= 1;
    if (tC) reg += 0x80;
    SetFlagsAfterShiftOp(reg);
}

void Z80::SLA_R(BYTE &reg)
{
    fC = (reg & 0x80) > 0;
    reg <<= 1;
    SetFlagsAfterShiftOp(reg);
}

void Z80::SRA_R(BYTE &reg)
{
    tC = (reg & 0x80) > 0;
    fC = (reg & 0x01) > 0;
    reg >>= 1;
    if (tC) reg += 0x80;
    SetFlagsAfterShiftOp(reg);
}

void Z80::SLL_R(BYTE &reg)
{
    fC = (reg & 0x80) > 0;
    reg <<= 1;
    reg += 1;
    SetFlagsAfterShiftOp(reg);
}

void Z80::SRL_R(BYTE &reg)
{
    fC = (reg & 0x01) > 0;
    reg >>= 1;
    SetFlagsAfterShiftOp(reg);
}

void Z80::ShiftOpIndHL(BYTE opCode)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = HL.Get();
        break;
    case 2:
        switch(opCode)
        {
        case 0x06:
            RLC_R(DR);
            break;
        case 0x0E:
            RRC_R(DR);
            break;
        case 0x16:
            RL_R(DR);
            break;
        case 0x1E:
            RR_R(DR);
            break;
        case 0x26:
            SLA_R(DR);
            break;
        case 0x2E:
            SRA_R(DR);
            break;
        case 0x36:
            SLL_R(DR);
            break;
        case 0x3E:
            SRL_R(DR);
            break;
        case 0x46:
            BIT_x_R(0, &DR);
            break;
        case 0x4E:
            BIT_x_R(1, &DR);
            break;
        case 0x56:
            BIT_x_R(2, &DR);
            break;
        case 0x5E:
            BIT_x_R(3, &DR);
            break;
        case 0x66:
            BIT_x_R(4, &DR);
            break;
        case 0x6E:
            BIT_x_R(5, &DR);
            break;
        case 0x76:
            BIT_x_R(6, &DR);
            break;
        case 0x7E:
            BIT_x_R(7, &DR);
            break;
        case 0x86:
            RES_x_R(0, &DR);
            break;
        case 0x8E:
            RES_x_R(1, &DR);
            break;
        case 0x96:
            RES_x_R(2, &DR);
            break;
        case 0x9E:
            RES_x_R(3, &DR);
            break;
        case 0xA6:
            RES_x_R(4, &DR);
            break;
        case 0xAE:
            RES_x_R(5, &DR);
            break;
        case 0xB6:
            RES_x_R(6, &DR);
            break;
        case 0xBE:
            RES_x_R(7, &DR);
            break;
        case 0xC6:
            SET_x_R(0, &DR);
            break;
        case 0xCE:
            SET_x_R(1, &DR);
            break;
        case 0xD6:
            SET_x_R(2, &DR);
            break;
        case 0xDE:
            SET_x_R(3, &DR);
            break;
        case 0xE6:
            SET_x_R(4, &DR);
            break;
        case 0xEE:
            SET_x_R(5, &DR);
            break;
        case 0xF6:
            SET_x_R(6, &DR);
            break;
        case 0xFE:
            SET_x_R(7, &DR);
            break;
        }
        mCycleType = MCycleType::WRITE;
        break;
    case 3:
        FinishInstruction();
        break;
    }
}

void Z80::BIT_x_R(int X, BYTE * R)
{
    fZ = (*R & (1 << X)) == 0;
    fH = true;
    fN = false;
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

void Z80::ADD_IDX_RR(Reg16 reg)
{
    switch(mCycle)
    {
    case 1:
        Z80::mCycleType = MCycleType::ALU;
        t16.Set((*IDX).Get());
        fC = t16.Get() + reg.Get() > 0xFFFF;
        fH = (t16.Get() & 0xFFF) + (reg.Get() & 0xFFF) > 0xFFF;
        break;
    case 2:
        t16H += *reg.H;
        if ((t16L + *reg.L) > 255) t16H += 1;
        t16L += *reg.L;
        break;
    case 3:
        fN = false;
        (*IDX).Set(t16.Get());
        FinishInstruction();
        break;
    }
}

void Z80::SBC_HL_RR(Reg16 reg)
{
  switch(mCycle)
  {
    case 1:
      tCV = fC ? 1 : 0;
      mCycleType = MCycleType::ALU;
      fC = reg.Get() + tCV > HL.Get();
      fP = fC;
      fH = (reg.Get() & 0xFFF)  + tCV > (HL.Get() & 0xFFF);
      break;
    case 2:
      H -= *reg.H;
      if (*reg.L + tCV > L) H--;
      L -= *reg.L + tCV;
      break;
    case 3:
      fN = true;
      fZ = H == 0 && L == 0;
      fS = (H & 0x80) > 0;
      FinishInstruction();
      break;
  }
}

void Z80::ADC_HL_RR(Reg16 reg)
{
    switch(mCycle)
    {
    case 1:
        tCV = fC ? 1 : 0;
        mCycleType = MCycleType::ALU;
        fC = HL.Get() + reg.Get() + tCV > 0xFFFF;
        fP = HL.Get() + reg.Get() + tCV > 0xFFFF;
        fH = (HL.Get() & 0xFFF) + (reg.Get() & 0xFFF) + tCV > 0xFFF;
        break;
    case 2:
        H += *reg.H;
        if ((L + *reg.L + tCV) > 255) H++;
        L += *reg.L + tCV;
        break;
    case 3:
        fN = false;
        fS = H & 0x80;
        fZ = H == 0 && L == 0;
        FinishInstruction();
        break;
    }
}

void Z80::NEG()
{
    t8 = A;
    A = 0;
    SUB_R(t8);
    fC = t8 != 0x00;
    fP = t8 == 0x80;
}

void Z80::LDI(bool R)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = HL.Get();
        break;
    case 2:
        mCycleType = MCycleType::WRITE;
        tAddr = DE.Get();
        break;
    case 3:
        mCycleType = MCycleType::ALU;
        DE.Set(DE.Get()+1);
        HL.Set(HL.Get()+1);
        BC.Set(BC.Get()-1);
        break;
    case 4:
        fP = BC.Get() != 0;
        fH = false;
        fN = false;
        if (!R || BC.Get() == 0)
            FinishInstruction();
        break;
    case 5:
        PC -= 2;
        FinishInstruction();
        break;
    }
}

void Z80::CPI(bool R)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = HL.Get();
        break;
    case 2:
        mCycleType = MCycleType::ALU;
        HL.Set(HL.Get() + 1);
        BC.Set(BC.Get() - 1);
        break;
    case 3:
        fH = (A & 0x0F) < (DR & 0x0F);
        fS = (A - DR) & 0x80;
        fZ = A == DR;
        fP = BC.Get() != 0;
        fN = true;
        break;
    case 4:
        if (!R || BC.Get() == 0 || A == DR) FinishInstruction();
        break;
    case 5:
        PC -= 2;
        FinishInstruction();
        break;
    }
}

void Z80::LDD(bool R)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = HL.Get();
        break;
    case 2:
        mCycleType = MCycleType::WRITE;
        tAddr = DE.Get();
        break;
    case 3:
        mCycleType = MCycleType::ALU;
        DE.Set(DE.Get()-1);
        HL.Set(HL.Get()-1);
        BC.Set(BC.Get()-1);
        break;
    case 4:
        fP = BC.Get() != 0;
        fH = false;
        fN = false;
        if (!R || BC.Get() == 0) FinishInstruction();
        break;
    case 5:
        PC -= 2;
        FinishInstruction();
        break;
    }
}

void Z80::CPD(bool R)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = HL.Get();
        break;
    case 2:
        mCycleType = MCycleType::ALU;
        HL.Set(HL.Get()-1);
        BC.Set(BC.Get()-1);
        break;
    case 3:
        fH = (A & 0xF) < (DR & 0xF);
        fS = ((A - DR) & 0x80) > 0;
        fZ = A == DR;
        break;
    case 4:
        fP = BC.Get() != 0;
        fN = true;
        if (!R || BC.Get() == 0 || A == DR) FinishInstruction();
        break;
    case 5:
        PC -= 2;
        FinishInstruction();
        break;
    }
}

void Z80::RLCA()
{
    fC = (A & 0x80) > 0;
    A <<= 1;
    if (fC) A++;
    fH = false;
    fN = false;
}

void Z80::RRCA()
{
    fC = (A & 0x01) > 0;
    A >>= 1;
    if (fC) A += 0x80;
    fH = false;
    fN = false;
}

void Z80::LD_A_I_RR(Reg16 reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = reg.Get();
        break;
    case 2:
        A = DR;
        FinishInstruction();
        break;
    }
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
    BYTE t = A;
    A = A_;
    A_ = t;
    EncodeF();
    t = F;
    F = F_;
    F_ = t;
    DecodeF();
}

void Z80::EX_DE_HL()
{
    BYTE t = D;
    D = H;
    H = t;
    t = E;
    E = L;
    L = t;
}

void Z80::EXX()
{
    BYTE t = B;
    B = B_;
    B_ = t;
    t = C;
    C = C_;
    C_ = t;
    t = D;
    D = D_;
    D_ = t;
    t = E;
    E = E_;
    E_ = t;
    t = H;
    H = H_;
    H_ = t;
    t = L;
    L = L_;
    L_ = t;
 }

void Z80::EX_HL_Ind_SP()
 {
     switch(mCycle)
     {
     case 1:
         mCycleType = MCycleType::READ;
         tAddr = SP.Get();
         break;
     case 2:
         *t16.L = DR;
         tAddr++;
         break;
     case 3:
         *t16.H = DR;
         DR = H;
         mCycleType = MCycleType::WRITE;
         break;
     case 4:
         tAddr--;
         DR = L;
         break;
     case 5:
         mCycleType = MCycleType::ALU;
         HL.Set(t16.Get());
         break;
     case 6:
         FinishInstruction();
         break;
     }
 }

void Z80::EX_IDX_Ind_SP()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = SP.Get();
        break;
    case 2:
        *t16.L = DR;
        tAddr++;
        break;
    case 3:
        *t16.H = DR;
        DR = *(*IDX).H;
        mCycleType = MCycleType::WRITE;
        break;
    case 4:
        tAddr--;
        DR = *(*IDX).L;
        break;
    case 5:
        mCycleType = MCycleType::ALU;
        (*IDX).Set(t16.Get());
        break;
    case 6:
        FinishInstruction();
        break;
    }
}

void Z80::JP_HL()
{
    PC = HL.Get();
}

void Z80::JP_IDX()
{
    PC = (*IDX).Get();
}

void Z80::DJNZ()
{
  switch(mCycle)
  {
    case 1:
      tAddr = PC;
      mCycleType = MCycleType::READ;
      break;
    case 2:
      tAddr++;
      PC = tAddr;
      B--;
      if (B == 0)
          FinishInstruction();
      else
          mCycleType = MCycleType::ALU;
      break;
    case 3:
      PC += (sbyte) DR;
      break;
    case 4:
      FinishInstruction();
      break;
  }
}

void Z80::RLA()
{
  tC = (A & 0x80) > 0;
  A <<= 1;
  if (fC) A++;
  fC = tC;
  fH = false;
  fN = false;
}

void Z80::RRA()
{
  tC = (A & 0x01) > 0;
  A >>= 1;
  if (fC) A += 0x80;
  fC = tC;
  fH = false;
  fN = false;
}

/*
    --------------------------------------------------------------------------------
    |           | C Flag  | HEX value in | H Flag | HEX value in | Number  | C flag|
    | Operation | Before  | upper digit  | Before | lower digit  | added   | After |
    |           | DAA     | (bit 7-4)    | DAA    | (bit 3-0)    | to byte | DAA   |
    |------------------------------------------------------------------------------|
    |           |    0    |     0-9      |   0    |     0-9      |   00    |   0   | Do nothing
    |   ADD     |    0    |     0-8      |   0    |     A-F      |   06    |   0   | Ok
    |           |    0    |     0-9      |   1    |     0-3      |   06    |   0   | Ok
    |   ADC     |    0    |     A-F      |   0    |     0-9      |   60    |   1   | Ok
    |           |    0    |     9-F      |   0    |     A-F      |   66    |   1   | Ok
    |   INC     |    0    |     A-F      |   1    |     0-3      |   66    |   1   | Ok
    |           |    1    |     0-2      |   0    |     0-9      |   60    |   1   | Ok
    |           |    1    |     0-2      |   0    |     A-F      |   66    |   1   | Ok
    |           |    1    |     0-3      |   1    |     0-3      |   66    |   1   | Ok
    |------------------------------------------------------------------------------|
    |   SUB     |    0    |     0-9      |   0    |     0-9      |   00    |   0   |
    |   SBC     |    0    |     0-8      |   1    |     6-F      |   FA    |   0   |
    |   DEC     |    1    |     7-F      |   0    |     0-9      |   A0    |   1   |
    |   NEG     |    1    |     6-F      |   1    |     6-F      |   9A    |   1   |
    --------------------------------------------------------------------------------
*/

void Z80::DAA()
{
    BYTE l = A & 0x0F;
    BYTE h = (A & 0xF0) >> 4;
    if (!fN)
    {
        if (!fC)
        {
            if (!fH)
            {
                if (h < 0x09 && l > 0x09) A += 0x06;
                else if (h > 0x09 && l < 0x0A) { A += 0x60; fC = true; }
                else if (h > 0x08 && l > 0x09) { A += 0x66; fC = true; }
            }
            else
            {
                if (h < 0x0A && l < 0x04) A += 0x06;
                else if (h > 0x09 && l < 0x04) { A += 0x66; fC = true; }
            }
        }
        else
        {
            if (!fH)
            {
                if (h < 0x03 && l < 0x0A) A += 0x60;
                else if (h <0x03 && l > 0x09) A += 0x66;
            }
            else
            {
                if (h < 0x03 && l < 0x03) A += 0x66;
            }
        }
    }
    else
    {
        if (!fC)
        {
            if (fH)
            {
                if (h < 0x09 && l > 0x05) A += 0xFA;
            }
        }
        else
        {
            if (!fH)
            {
                if (h > 0x06 && l < 0x0A) A += 0xA0;
            }
            else
            {
                if (h > 0x05 && l > 0x05) A += 0x9A;
            }
        }
    }

/*
  BYTE correction = 0;
  if (A > 0x99 || fC.Get())
  {
    correction = 0x60;
    fC.Set(true);
  }
  else
    fC.Set(false);
  if ((A & 0xF) > 9 || fH.Get())
    correction |= 0x6;
  if (fN.Get())
  {
    fH.Set(correction > (A & 0xF));
    fP.Set(correction > A);
    A -= correction;
  }
  else
  {
    fH.Set(correction > 0xF - (A & 0xF));
    fP.Set(A + correction > 0xFF);
    A += correction;
  }
  fZ.Set(A == 0);
  fS.Set((A & 0x80) > 0);
  */
}

void Z80::CPL()
{
  A ^= 0xFF;
  fH = true;
  fN = true;
}

void Z80::INC_Ind_HL()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = HL.Get();
        break;
    case 2:
        INC_R(DR);
        mCycleType = MCycleType::WRITE;
        break;
    case 3:
        FinishInstruction();
        break;
    }
}

void Z80::DEC_Ind_HL()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = HL.Get();
        break;
    case 2:
        DEC_R(DR);
        mCycleType = MCycleType::WRITE;
        break;
    case 3:
        FinishInstruction();
        break;
    }
}

void Z80::LD_Ind_HL_n()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = PC;
        break;
    case 2:
        tAddr++;
        PC = tAddr;
        tAddr = HL.Get();
        mCycleType = MCycleType::WRITE;
        break;
    case 3:
        FinishInstruction();
        break;
    }
}

void Z80::SCF()
{
    fC = true;
    fN = false;
    fH = false;
}

void Z80::CCF()
{
    fH = fC;
    fC = !fC;
    fN = false;
}

void Z80::LD_SP_HL()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::ALU;
        SP.Set(HL.Get());
        break;
    case 2:
        FinishInstruction();
        break;
    }
}

void Z80::LD_SP_IDX()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::ALU;
        SP.Set((*IDX).Get());
        break;
    case 2:
        FinishInstruction();
        break;
    }
}

void Z80::IN_R_PortBC(BYTE &reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::IN;
        tAddr = BC.Get();
        break;
    case 2:
        mCycleType = MCycleType::ALU;
        reg = DR;
        break;
    case 3:
        fS = (DR & 0x80) > 0;
        fZ = DR == 0;
        fH = false;
        fP = GetParity(DR);
        fN = false;
        FinishInstruction();
        break;
    }
}

void Z80::OUT_PortBC_R(BYTE &reg)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::OUT;
        tAddr = BC.Get();
        DR = reg;
        break;
    case 2:
        mCycleType = MCycleType::ALU;
        break;
    case 3:
        FinishInstruction();
        break;
    }
}

void Z80::LD_A_I()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::ALU;
        A = I;
        break;
    case 2:
        fZ = A == 0;
        fS = A & 0x80;
        fH = false;
        fN = false;
        fP = InterruptEnable;
        FinishInstruction();
        break;
    }
}

void Z80::LD_I_A()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::ALU;
        I = A;
        break;
    case 2:
        FinishInstruction();
        break;
    }
}

void Z80::LD_A_R()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::ALU;
        A = R;
        break;
    case 2:
        fP = InterruptEnable;
        fZ = A == 0;
        fS = A & 0x80;
        fH = false;
        fN = false;
        FinishInstruction();
        break;
    }
}

void Z80::LD_R_A()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::ALU;
        R = A;
        break;
    case 2:
        FinishInstruction();
        break;
    }
}

void Z80::RLD()
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = HL.Get();
        break;
    case 2:
        t8 = DR;
        DR = (BYTE)((DR << 4) + (A & 0x0F));
        mCycleType = MCycleType::WRITE;
        break;
    case 3:
        mCycleType = MCycleType::ALU;
        A = (BYTE)((A & 0xF0) + (t8 >> 4));
        break;
    case 4:
        fP = (A & 0x01) == 0;
        fZ = A == 0;
        fS = (A & 0x80) > 0;
        fH = false;
        fN = false;
        FinishInstruction();
        break;
    }
}

void Z80::RRD()
{
  switch(mCycle)
  {
    case 1:
      mCycleType = MCycleType::READ;
        tAddr = HL.Get();
    break;
    case 2:
      t8 = DR;
      DR = (BYTE)((DR >> 4) + (A << 4));
      mCycleType = MCycleType::WRITE;
    break;
    case 3:
      mCycleType = MCycleType::ALU;
      A = (BYTE)((A & 0xF0) + (t8 & 0x0F));
    break;
    case 4:
        fP = (A & 0x01) == 0;
        fZ = A == 0;
        fS = (A & 0x80) > 0;
        fH = false;
        fN = false;
      FinishInstruction();
    break;
  }
}

void Z80::IM(int mode)
{
    InterruptMode = mode;
    idMode = IDMode::BASIC;
}

void Z80::INI(bool R, bool dir)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::IN;
        tAddr = BC.Get();
        break;
    case 2:
        mCycleType = MCycleType::WRITE;
        tAddr = HL.Get();
        break;
    case 3:
        mCycleType = MCycleType::ALU;
        if (dir)
            HL.Set(HL.Get() + 1);
        else
            HL.Set(HL.Get() - 1);
        B--;
        break;
    case 4:
        fS = (DR & 0x80) > 0;
        fZ = B == 0;
        fN = true;
        if (!R || B == 0)
            FinishInstruction();
        break;
    case 5:
        PC -= 2;
        FinishInstruction();
        break;
    }
}

void Z80::OUTI(bool R, bool dir)
{
    switch(mCycle)
    {
    case 1:
        mCycleType = MCycleType::READ;
        tAddr = HL.Get();
        break;
    case 2:
        mCycleType = MCycleType::OUT;
        tAddr = BC.Get();
        break;
    case 3:
        mCycleType = MCycleType::ALU;
        if (dir)
            HL.Set(HL.Get() + 1);
        else
            HL.Set(HL.Get() - 1);
        B--;
        break;
    case 4:
        fS = (DR & 0x80) > 0;
        fZ = B == 0;
        fN = true;
        if (!R || B == 0)
            FinishInstruction();
        break;
    case 5:
        PC -= 2;
        FinishInstruction();
        break;
    }
}


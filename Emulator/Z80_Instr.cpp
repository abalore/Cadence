#include "Headers/Z80.h"

void Z80::INC_R(BYTE &reg)
{
    fH.Set((reg & 0xF) == 0xF);
    fP.Set(reg == 0x7F);
    reg++;
    fS.Set(reg & 0x80);
    fZ.Set(reg == 0);
    fN.Set(false);
}

void Z80::DEC_R(BYTE &reg)
{
    fH.Set((reg & 0xF) == 0x0);
    fP.Set(reg == 0x80);
    reg--;
    fS.Set(reg & 0x80);
    fZ.Set(reg == 0);
    fN.Set(true);
}

void Z80::JR(bool condition)
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
        fC.Set(HL.Get() + reg.Get() > 0xFFFF);
        fH.Set((HL.Get() & 0xFFF) + (reg.Get() & 0xFFF) > 0xFFF);
        break;
    case 2:
        H += *reg.H;
        if ((L + *reg.L) > 255) H++;
        L += *reg.L;
        break;
    case 3:
        fN.Set(false);
        FinishInstruction();
        break;
    }
}

void Z80::ADD_A_v(BYTE v)
{
    int signedResult = (sbyte)A + (sbyte)v;
    fP.Set((signedResult > 127) || (signedResult < -128));
    fC.Set(A + v > 255);
    fH.Set(((A & 0xF) + (v & 0xF)) > 0xF);
    A += v;
    fS.Set((A & 0x80) > 0);
    fZ.Set(A == 0);
    fN.Set(false);
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
        ADD_A_v(DR);
        PC++;
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
        ADC_A_v(DR);
        PC++;
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
        SUB_A_v(DR);
        PC++;
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
        SBC_A_v(DR);
        PC++;
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
        AND_A_v(DR);
        PC++;
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
        XOR_A_v(DR);
        PC++;
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
        OR_A_v(DR);
        PC++;
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
        CP_v(DR);
        PC++;
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
      ADD_R(DR);
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
    ADD_A_v((BYTE)(v + (fC.Get() ? 1 : 0)));
}

void Z80::ADC_R(BYTE &reg)
{
    ADC_A_v(reg);
}

void Z80::SUB_A_v(BYTE v)
{
  int signedResult = (sbyte)A - (sbyte)v;
  fP.Set((signedResult > 127) || (signedResult < -128));
  fH.Set((A & 0xF) < (v & 0xF));
  fC.Set(A < v);
  A -= v;
  fS.Set((A & 0x80) > 0);
  fZ.Set(A == 0);
  fN.Set(true);
}

void Z80::SUB_R(BYTE &reg)
{
  SUB_A_v(reg);
}

void Z80::SBC_A_v(BYTE v)
{
    SUB_A_v((BYTE)(v + (fC.Get() ? 1 : 0)));
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
    fP.Set(GetParity(b));
    fS.Set((b & 0x80) > 0);
    fZ.Set(b == 0);
    fN.Set(false);
    fC.Set(false);
}

void Z80::SetFlagsAfterShiftOp(BYTE b)
{
    fP.Set((b & 0x01) == 0);
    fS.Set((b & 0x80) > 0);
    fZ.Set(b == 0);
    fH.Set(false);
    fN.Set(false);
}

void Z80::SetFlagsAfterBIT()
{
    fH.Set(true);
    fN.Set(false);
}

void Z80::AND_A_v(BYTE v)
{
    A &= v;
    fH.Set(true);
    SetFlagsAfterLogicalOp(A);
}

void Z80::AND_R(BYTE &reg)
{
  AND_A_v(reg);
}

void Z80::XOR_A_v(BYTE v)
{
  A ^= v;
  fH.Set(false);
  SetFlagsAfterLogicalOp(A);
}

void Z80::XOR_R(BYTE &reg)
{
  XOR_A_v(reg);
}

void Z80::OR_A_v(BYTE v)
{
  A |= v;
  fH.Set(false);
  SetFlagsAfterLogicalOp(A);
}

void Z80::OR_R(BYTE &reg)
{
  OR_A_v(reg);
}

void Z80::CP_v(BYTE v)
{
  t8 = A;
  SUB_A_v(v);
  A = t8;
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
    fC.Set((reg & 0x80) > 0);
    reg <<= 1;
    if (fC.Get()) reg++;
    SetFlagsAfterShiftOp(reg);
}

void Z80::RRC_R(BYTE &reg)
{
    fC.Set((reg & 0x01) > 0);
    reg >>= 1;
    if (fC.Get()) reg += 0x80;
    SetFlagsAfterShiftOp(reg);
}

void Z80::RL_R(BYTE &reg)
{
    bool c = fC.Get();
    fC.Set((reg & 0x80) > 0);
    reg <<= 1;
    if (c) reg++;
    SetFlagsAfterShiftOp(reg);
}

void Z80::RR_R(BYTE &reg)
{
    bool c = fC.Get();
    fC.Set((reg & 0x01) > 0);
    reg >>= 1;
    if (c) reg += 0x80;
    SetFlagsAfterShiftOp(reg);
}

void Z80::SLA_R(BYTE &reg)
{
    fC.Set((reg & 0x80) > 0);
    reg <<= 1;
    SetFlagsAfterShiftOp(reg);
}

void Z80::SRA_R(BYTE &reg)
{
    bool c = (reg & 0x80) > 0;
    fC.Set((reg & 0x01) > 0);
    reg >>= 1;
    if (c) reg += 0x80;
    SetFlagsAfterShiftOp(reg);
}

void Z80::SLL_R(BYTE &reg)
{
    fC.Set((reg & 0x80) > 0);
    reg <<= 1;
    reg += 1;
    SetFlagsAfterShiftOp(reg);
}

void Z80::SRL_R(BYTE &reg)
{
    fC.Set((reg & 0x01) > 0);
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
    fZ.Set((*R & (1 << X)) == 0);
    SetFlagsAfterBIT();
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
        t16.Set((*IDX).Get());
        fC.Set(t16.Get() + reg.Get() > 0xFFFF);
        fH.Set((t16.Get() & 0xFFF) + (reg.Get() & 0xFFF) > 0xFFF);
        break;
    case 2:
        t16H += *reg.H;
        if ((t16L + *reg.L) > 255) t16H += 1;
        t16L += *reg.L;
        break;
    case 3:
        fN.Set(false);
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
      mCycleType = MCycleType::ALU;
      t16.Set(reg.Get());
      if (fC.Get()) t16.Set(t16.Get() + 1);
      fC.Set(t16.Get() > HL.Get());
      fH.Set((t16.Get() & 0xFFF) > (HL.Get() & 0xFFF));
      break;
    case 2:
      H -= *t16.H;
      if (*t16.L > L) H--;
      L -= *t16.L;
      break;
    case 3:
      fN.Set(true);
      fZ.Set(HL.Get() == 0);
      fS.Set((H & 0x80) > 0);
      FinishInstruction();
      break;
  }
}

void Z80::ADC_HL_RR(Reg16 reg)
{
    if (mCycle == 1)
    {
        t16.Set(reg.Get());
        if (fC.Get()) t16.Set(t16.Get() + 1);
    }
    ADD_HL_RR(t16);
}

void Z80::NEG()
{
    t8 = A;
    A = 0;
    SUB_R(t8);
    fC.Set(t8 != 0x00);
    fP.Set(t8 == 0x80);
    idMode = IDMode::BASIC;
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
        fP.Set(BC.Get() != 0);
        fH.Set(false);
        fN.Set(false);
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
        HL.Set(HL.Get()+1);
        BC.Set(BC.Get()-1);
        break;
    case 3:
        fH.Set((A & 0xF) < (DR & 0xF));
        fS.Set(((A - DR) & 0x80) > 0);
        fZ.Set(A == DR);
        fP.Set(BC.Get() != 0);
        fN.Set(true);
        break;
    case 4:
        if (!R || BC.Get() == 0) FinishInstruction();
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
        fP.Set(BC.Get() != 0);
        fH.Set(false);
        fN.Set(false);
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
        fH.Set((A & 0xF) < (DR & 0xF));
        fS.Set(((A - DR) & 0x80) > 0);
        fZ.Set(A == DR);
        break;
    case 4:
        fP.Set(BC.Get() != 0);
        fN.Set(true);
        if (!R || BC.Get() == 0) FinishInstruction();
        break;
    case 5:
        PC -= 2;
        FinishInstruction();
        break;
    }
}

void Z80::RLCA()
{
    fC.Set((A & 0x80) > 0);
    A <<= 1;
    if (fC.Get()) A++;
    fH.Set(false);
    fN.Set(false);
}

void Z80::RRCA()
{
    fC.Set( (A & 0x01) > 0);
    A >>= 1;
    if (fC.Get()) A += 0x80;
    fH.Set(false);
    fN.Set(false);
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

void Z80::EX_AF_AF_()
{
    BYTE t = A;
    A = A_;
    A_ = t;
    t = F;
    F = F_;
    F_ = t;
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
  bool c = (A & 0x80) > 0;
  A <<= 1;
  if (fC.Get()) A++;
  fC.Set(c);
  fH.Set(false);
  fN.Set(false);
}

void Z80::RRA()
{
  bool c = (A & 0x01) > 0;
  A >>= 1;
  if (fC.Get()) A += 0x80;
  fC.Set(c);
  fH.Set(false);
  fN.Set(false);
}

void Z80::DAA()
{
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
}

void Z80::CPL()
{
  A ^= 0xFF;
  fH.Set(true);
  fN.Set(true);
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
    fC.Set(true);
}

void Z80::CCF()
{
    fC.Set(!fC.Get());
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
        fS.Set((DR & 0x80) > 0);
        fZ.Set(DR == 0);
        fH.Set(false);
        fP.Set(GetParity(DR));
        fN.Set(false);
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
        DR = (BYTE)((DR << 4) + (A & 0x0F));
        mCycleType = MCycleType::WRITE;
        break;
    case 3:
        mCycleType = MCycleType::ALU;
        A = (BYTE)((A & 0xF0) + (DR >> 4));
        break;
    case 4:
        fP.Set((A & 0x01) == 0);
        fZ.Set(A == 0);
        fS.Set((A & 0x80) > 0);
        fH.Set(false);
        fN.Set(false);
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
      DR = (BYTE)((DR >> 4) + (A << 4));
      mCycleType = MCycleType::WRITE;
    break;
    case 3:
      mCycleType = MCycleType::ALU;
      A = (BYTE)((A & 0xF0) + (DR & 0x0F));
    break;
    case 4:
        fP.Set((A & 0x01) == 0);
        fZ.Set(A == 0);
        fS.Set((A & 0x80) > 0);
        fH.Set(false);
        fN.Set(false);
      FinishInstruction();
    break;
  }
}

void Z80::IM(int mode)
{
    switch(mode)
    {
    case 0:
        ////////////////////////////////
        break;
    case 1:
        ///////////////////////////////////
        break;
    case 2:
        //////////////////////////////
        break;
    }
    idMode = IDMode::BASIC;
}


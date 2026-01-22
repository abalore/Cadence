#include "Headers/Z80.h"
#include "Headers/CPC.h"
#include <mutex>

using namespace std;

mutex Z80::debugStringLock;

BYTE Z80::tCycle = 1;
BYTE Z80::mCycle = 1;
bool Z80::MREQ = true;
bool Z80::RD = true;
bool Z80::WR = true;
bool Z80::IORQ = true;
// bool Z80::WAIT = true;
bool Z80::RFSH = true;
bool Z80::M1 = true;
word Z80::PC = 0;
Reg16 Z80::SP(&SPH, &SPL);
Reg16 Z80::AF(&A, &F);
Reg16 Z80::BC(&B, &C);
Reg16 Z80::DE(&D, &E);
Reg16 Z80::HL(&H, &L);
Reg16 Z80::IX(&IXH, &IXL);
Reg16 Z80::IY(&IYH, &IYL);
Reg16 Z80::AF_(&A_,&F_);
Reg16 Z80::BC_(&B_, &C_);
Reg16 Z80::DE_(&C_, &D_);
Reg16 Z80::HL_(&H_, &L_);
Flag Z80::fS(&F, 0x80);
Flag Z80::fZ(&F, 0x40);
Flag Z80::f5(&F, 0x20);
Flag Z80::fH(&F, 0x10);
Flag Z80::f3(&F, 0x8);
Flag Z80::fP(&F, 0x4);
Flag Z80::fN(&F, 0x2);
Flag Z80::fC(&F, 0x1);
BYTE Z80::A;
BYTE Z80::F;
BYTE Z80::B;
BYTE Z80::C;
BYTE Z80::D;
BYTE Z80::E;
BYTE Z80::H;
BYTE Z80::L;
BYTE Z80::IXH;
BYTE Z80::IXL;
BYTE Z80::IYH;
BYTE Z80::IYL;
BYTE Z80::A_;
BYTE Z80::F_;
BYTE Z80::B_;
BYTE Z80::C_;
BYTE Z80::D_;
BYTE Z80::E_;
BYTE Z80::H_;
BYTE Z80::L_;
BYTE Z80::t16H;
BYTE Z80::t16L;
BYTE Z80::SPH;
BYTE Z80::SPL;
BYTE Z80::I;
BYTE Z80::R;
BYTE Z80::IR;
BYTE Z80::DR;
IDMode Z80::idMode = IDMode::BASIC;
MCycleType Z80::mCycleType = MCycleType::FETCH;
BYTE Z80::t8 = 0;
Reg16 Z80::t16(&t16H, &t16L);
sbyte Z80::index = 0;
BYTE Z80::tByte = 0;
BYTE Z80::opCode = 0;
Reg16 *Z80::IDX = &IX;
word Z80::tAddr = 0;
    
void Z80::Reset()
{
    PC = 0;
}

void Z80::Step()
{
    switch(idMode)
    {
        case IDMode::BASIC:
            Step_basic();
            break;
        case IDMode::MISC:
            Step_misc();
            break;
        case IDMode::BIT:
            Step_CB();
            break;
        case IDMode::IDX:
            Step_IDX();
            break;
        case IDMode::IDX2:
            Step_IDX_2();
            break;
        case IDMode::IDXBIT:
            Step_IDX_CB();
            break;
    }
}

void Z80::ProcessFETCH(bool edge)
{
    if (edge)
    {
        switch (tCycle)
        {
            case 1:
                CPC::AddressBUS = PC;
                RFSH = true;
                M1 = false;
                break;
            case 3:
                M1 = true;
                IR = CPC::DataBUS;
                RD = true;
                RFSH = false;
                MREQ = true;
                PC++;
                break;
        }
    }
    else
    {
        switch (tCycle)
        {
            case 1:
                MREQ = false;
                RD = false;
                break;
            case 3:
                MREQ = false;
                break;
            case 4:
                MREQ = true;
                break;
        }
    }
}

void Z80::ProcessREAD(bool edge)
{
    if (edge)
    {
        switch (tCycle)
        {
            case 1:
                CPC::AddressBUS = tAddr;
                break;
            case 3:
                DR = CPC::DataBUS;
                break;
        }
    }
    else
    {
        switch (tCycle)
        {
            case 1:
                MREQ = false;
                RD = false;
                break;
            case 3:
                RD = true;
                MREQ = true;
                break;
        }
    }
}

void Z80::ProcessWRITE(bool edge)
{
    if (edge)
    {
        switch (tCycle)
        {
            case 1:
                CPC::AddressBUS = tAddr;
                break;
        }
    }
    else
    {
        switch (tCycle)
        {
            case 1:
                MREQ = false;
                CPC::DataBUS = DR;
                break;
            case 2:
                WR = false;
                break;
            case 3:
                WR = true;
                MREQ = true;
                break;
        }
    }
}

void Z80::ProcessIN(bool edge)
{
    if (edge)
    {
        switch (tCycle)
        {
            case 1:
                CPC::AddressBUS = tAddr;
                break;
            case 2:
                IORQ = false;
                RD = false;
                break;
            case 4:
                DR = CPC::DataBUS;
                break;
        }
    }
    else
    {
        switch (tCycle)
        {
            case 4:
                RD = true;
                IORQ = true;
                break;
        }
    }
}

void Z80::ProcessOUT(bool edge)
{
    if (edge)
    {
        switch (tCycle)
        {
            case 1:
                CPC::AddressBUS = tAddr;
                break;
            case 2:
                IORQ = false;
                WR = false;
                break;
        }
      }
      else
      {
        switch (tCycle)
        {
            case 1:
                CPC::DataBUS = DR;
                break;
            case 4:
                WR = true;
                IORQ = true;
                break;
        }
    }
}
    
void Z80::ClockEdge(bool edge)
{
    switch (mCycleType)
    {
        case MCycleType::FETCH:
            ProcessFETCH(edge);
            break;
        case MCycleType::READ:
            ProcessREAD(edge);
            break;
        case MCycleType::WRITE:
            ProcessWRITE(edge);
            break;
        case MCycleType::IN:
            ProcessIN(edge);
            break;
        case MCycleType::OUT:
            ProcessOUT(edge);
            break;
        default:
            break;
    }
    if (!edge)
    {
        if (tCycle == 4)
        {
            Step();
            tCycle = 1;
            if (mCycleType == MCycleType::FETCH)
            {
                mCycle = 1;
            }
            else
                mCycle++;
        }
        else
            tCycle++;
    }
}

BYTE Z80::ReadMEM(int address)
{
    return CPC::InternalRAM->MEM[address];
}

void Z80::WriteMEM(int address, BYTE value)
{
    CPC::InternalRAM->MEM[address] = value;
}

void Z80::ReadMEMHL()
{
    t8 = ReadMEM(HL.Get());
}

void Z80::ReadMEMHLReg(BYTE &reg)
{
    reg = ReadMEM(HL.Get());
}

BYTE Z80::ReadMEMHLDirect()
{
    return ReadMEM(HL.Get());
}

void Z80::WriteMEMHL()
{
    WriteMEM(HL.Get(), t8);
}

void Z80::WriteMEMHLReg(BYTE reg)
{
    WriteMEM(HL.Get(), reg);
}

void Z80::ReadMEMIDX()
{
    t8 = ReadMEM(IDX->Get() + index);
}

void Z80::WriteMEMIDX()
{
    WriteMEM(IDX->Get() + index, t8);
}

void Z80::ReadMEMIDX16()
{
    *t16.L = ReadMEM(IDX->Get());
    *t16.H = ReadMEM(IDX->Get() + 1);
}

BYTE Z80::ReadMEMPCInc()
{
    return ReadMEM(PC++);
}

void Z80::FinishInstruction()
{
    mCycleType = MCycleType::FETCH;
   idMode = IDMode::BASIC;
}


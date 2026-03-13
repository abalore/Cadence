#include "Z80.h"
#include "CPC.h"
#include "CRTC.h"
#include "PPI.h"
#include "PSG.h"
#include "CRTC.h"
#include "FDC.h"
#include "GateArray.h"
#include "Emulator.h"

BYTE Z80::tCycle = 1;
BYTE Z80::mCycle = 1;
bool Z80::MREQ, Z80::IORQ, Z80::RD, Z80::WR;
word Z80::PC = 0;
word Z80::SP = 0;
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
bool Z80::fS, Z80::fZ, Z80::f5, Z80::fH, Z80::f3, Z80::fP, Z80::fN, Z80::fC;
BYTE Z80::A, Z80::F, Z80::B, Z80::C, Z80::D, Z80::E, Z80::H, Z80::L;
BYTE Z80::IXH, Z80::IXL, Z80::IYH, Z80::IYL;
BYTE Z80::A_, Z80::F_, Z80::B_, Z80::C_, Z80::D_, Z80::E_, Z80::H_, Z80::L_;
BYTE Z80::t16H;
BYTE Z80::t16L;
BYTE Z80::SPH;
BYTE Z80::SPL;
BYTE Z80::I, Z80::R;
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
word Z80::AR = 0;
bool Z80::IFF1 = false;
bool Z80::IFF2 = false;
bool Z80::InterruptRequest = true;
BYTE Z80::im = 0;
bool Z80::stopPoint = false;
BYTE Z80::t_cp = 0;
bool Z80::halted = false;
bool Z80::tC = false;
int Z80::tCV = 0;
BYTE Z80::t;
word Z80::w1;
word Z80::w2;
word Z80::w3;
int Z80::i1;
int Z80::i2;
int Z80::i3;
short Z80::s1;
dword Z80::nops;
bool Z80::EIRequest;
bool Z80::intAlign;

void Z80::Reset()
{
    MREQ = true;
    IORQ = true;
    RD = true;
    WR = true;
    PC = 0;
    SP = 0;
    I = 0;
    R = 0;
    im = 0;
    mCycle = 1;
    nops = 0;
    idMode = IDMode::BASIC;
    mCycleType = MCycleType::FETCH;
    IFF1 = false;
    IFF2 = false;
    InterruptRequest = true;
    stopPoint = false;
    halted = false;
    EIRequest = false;
}

void Z80::ProcessINT()
{
    R = (R & 0x80) | ((R + 1) & 0x7F);
    if (im == 1) IR = 0xFF;
}

void Z80::ProcessFETCH()
{
    R = (R & 0x80) | ((R + 1) & 0x7F);
    if (halted)
        IR = 0x00;
    else
    {
        AR = PC;
        ProcessREAD();
        IR = DR;
        PC++;
    }
    intAlign = false;
}

void Z80::ProcessREAD()
{
    DR = CPC::GetByteAt(AR);
}

void Z80::ProcessWRITE()
{
    CPC::SetByteAt(AR, DR);
}

void Z80::ProcessIN()
{
    if (!(AR & 0x4000)) CRTC::RD();
    else if (!(AR & 0x0800)) PPI::RD();
    else if (!(AR & 0x0480) && CPC::cpcType != CPCType::CPC464) FDC::RD();
}

void Z80::ProcessOUT()
{
    if (!(AR & 0x8000)) GateArray::WR();
    else if (!(AR & 0x4000)) CRTC::WR();
    else if (!(AR & 0x2000)) CPC::SelectROM(DR);
    else if (!(AR & 0x0800)) PPI::WR();
    else if (!(AR & 0x0480) && CPC::cpcType != CPCType::CPC464) FDC::WR();
}

void Z80::Clock()
{
    switch (mCycleType)
    {
    case MCycleType::INT: ProcessINT(); break;
    case MCycleType::FETCH: ProcessFETCH(); break;
    case MCycleType::READ: ProcessREAD(); break;
    case MCycleType::WRITE: ProcessWRITE(); break;
    case MCycleType::IN: ProcessIN(); break;
    case MCycleType::OUT: ProcessOUT(); break;
    case MCycleType::ALU: break;
    }
    switch(idMode)
    {
    case IDMode::BASIC: Step_basic(); break;
    case IDMode::MISC: Step_misc(); break;
    case IDMode::BIT: Step_CB(); break;
    case IDMode::IDX: Step_IDX(); break;
    case IDMode::IDXBIT: Step_IDX_CB(); break;
    case IDMode::INTEXEC: Step_Int_Exec(); break;
    }
    nops++;
}

void Z80::Clock2()
{
    if (mCycleType == MCycleType::FETCH)
    {
        mCycle = 1;
        if (idMode == IDMode::BASIC)
        {
            if (!InterruptRequest && IFF1)
            {
                IFF1 = false;
                IFF2 = false;
                InterruptRequest = true;
                mCycleType = MCycleType::INT;
                if (halted)
                {
                    halted = false;
                    PC++;
                }
                idMode = IDMode::INTEXEC;
                EIRequest = false;
            }
            else
            {
                stopPoint = true;
                if (EIRequest)
                {
                    IFF1 = true;
                    IFF2 = true;
                    EIRequest = false;
                }
            }
        }
    }
    else
        mCycle++;
}




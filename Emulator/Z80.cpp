#include "Headers/Z80.h"
#include "Headers/CPC.h"
#include "Headers/CRTC.h"
#include "Headers/PPI.h"
#include "Headers/PSG.h"
#include "Headers/CRTC.h"
#include "Headers/FDC.h"
#include "Headers/GateArray.h"
#include "Headers/ROMSelector.h"
#include "Headers/Emulator.h"

using namespace std;

BYTE Z80::tCycle = 1;
BYTE Z80::mCycle = 1;
bool Z80::MREQ = true;
bool Z80::RD = true;
bool Z80::WR = true;
bool Z80::IORQ = true;
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
bool Z80::fS;
bool Z80::fZ;
bool Z80::f5;
bool Z80::fH;
bool Z80::f3;
bool Z80::fP;
bool Z80::fN;
bool Z80::fC;
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
word Z80::AR = 0;
bool Z80::InterruptEnable = false;
bool Z80::InterruptRequest = true;
BYTE Z80::InterruptMode = 0;
bool Z80::stopPoint = false;
BYTE Z80::t_cp = 0;
bool Z80::halted = false;
bool Z80::tC = false;
int Z80::tCV = 0;

void Z80::Init()
{
    mCycle = 1;
    PC = 0;
    idMode = IDMode::BASIC;
    mCycleType = MCycleType::FETCH;
    InterruptEnable = true;
    InterruptRequest = true;
    InterruptMode = 0;
    stopPoint = false;
    halted = false;
}

void Z80::ProcessINT()
{
    if (InterruptMode == 1) IR = 0xFF;
}

void Z80::ProcessFETCH()
{
    t8 = R & 0x80;
    R++;
    R = (R & 0x7F) + t8;
    AR = PC;
    ProcessREAD();
    if (halted)
        IR = 0x00;
    else
    {
        IR = DR;
        PC++;
    }
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
    if ((AR & 0x4000) == 0) CRTC::RD();
    else if ((AR & 0x0800) == 0) { PSG::RD(); PPI::RD(); }
    else if ((AR & 0x0480) == 0 && Emulator::cpcType != CPCType::CPC464) FDC::RD();
}

void Z80::ProcessOUT()
{
    if ((AR & 0x8000) == 0) GateArray::WR();
    else if ((AR & 0x4000) == 0) CRTC::WR();
    else if ((AR & 0x2000) == 0) ROMSelector::WR();
    else if ((AR & 0x0800) == 0) { PPI::WR(); PSG::WR(); }
    else if ((AR & 0x0480) == 0 && Emulator::cpcType != CPCType::CPC464) FDC::WR();
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
    case IDMode::IDX2: Step_IDX_2(); break;
    case IDMode::IDXBIT: Step_IDX_CB(); break;
    case IDMode::INTEXEC: Step_Int_Exec(); break;
    }
    if (mCycleType == MCycleType::FETCH)
    {
        mCycle = 1;
        if (idMode == IDMode::BASIC)
        {
            stopPoint = true;
            if (!InterruptRequest && InterruptEnable)
            {
                InterruptEnable = false;
                InterruptRequest = true;
                //IR = 0xFF;
                mCycleType = MCycleType::INT;
                halted = false;
                idMode = IDMode::INTEXEC;
            }
        }
    }
    else
        mCycle++;
}

void Z80::FinishInstruction()
{
    mCycleType = MCycleType::FETCH;
    idMode = IDMode::BASIC;
}


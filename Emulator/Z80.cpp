#include "Headers/Z80.h"
#include "Headers/CPC.h"

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
word Z80::tAddr = 0;
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
    tCycle = 1;
    mCycle = 1;
    MREQ = true;
    RD = true;
    WR = true;
    IORQ = true;
    M1 = true;
    PC = 0;
    idMode = IDMode::BASIC;
    mCycleType = MCycleType::FETCH;
    InterruptEnable = true;
    InterruptRequest = true;
    InterruptMode = 0;
    stopPoint = false;
    halted = false;
}

void Z80::Step()
{
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
}

void Z80::ProcessINT()
{
    if (tCycle == 3 && InterruptMode == 1) IR = 0xFF;
}

void Z80::ProcessFETCH()
{
    switch (tCycle)
    {
    case 1:
        if (idMode == IDMode::BASIC)
        {
            stopPoint = true;
            M1 = false;
        }
        t8 = R & 0x80;
        R++;
        R = (R & 0x7F) + t8;
        CPC::AddressBUS = PC;
        MREQ = false;
        RD = false;
        break;
    case 3:
        M1 = true;
        RD = true;
        MREQ = true;
        if (halted)
            IR = 0x00;
        else
        {
            IR = CPC::DataBUS;
            PC++;
        }

        MREQ = false;
        break;
    case 4:
        MREQ = true;
        break;
    }
}

void Z80::ProcessREAD()
{
    switch (tCycle)
    {
    case 1:
        CPC::AddressBUS = tAddr;
        MREQ = false;
        RD = false;
        break;
    case 3:
        DR = CPC::DataBUS;
        RD = true;
        MREQ = true;
        break;
    }
}

void Z80::ProcessWRITE()
{
    switch (tCycle)
    {
    case 1:
        CPC::AddressBUS = tAddr;
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

void Z80::ProcessIN()
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
        RD = true;
        IORQ = true;
        break;
    }
}

void Z80::ProcessOUT()
{
    switch (tCycle)
    {
    case 1:
        CPC::AddressBUS = tAddr;
        CPC::DataBUS = DR;
        break;
    case 2:
        IORQ = false;
        WR = false;
        break;
    case 4:
        WR = true;
        IORQ = true;
        break;
    }
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
    if (tCycle == 4)
    {
        Step();
        tCycle = 1;
        if (mCycleType == MCycleType::FETCH)
        {
            mCycle = 1;
            if (!InterruptRequest && InterruptEnable && idMode == IDMode::BASIC)
            {
                InterruptEnable = false;
                InterruptRequest = true;
                IR = 0xFF;
                mCycleType = MCycleType::INT;
                halted = false;
                idMode = IDMode::INTEXEC;
            }
        }
        else
            mCycle++;
    }
    else
        tCycle++;
}

void Z80::FinishInstruction()
{
    mCycleType = MCycleType::FETCH;
    idMode = IDMode::BASIC;
}


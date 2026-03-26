#include "Z80.h"
#include "CPC.h"
#include "GateArray.h"
#include "CRTC.h"
#include "PPI.h"
#include "FDC.h"

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
bool Z80::M1;
bool Z80::WAIT;
bool Z80::lastMCycle;
bool Z80::lastTCycle;

void Z80::Reset()
{
    WAIT = true;
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

bool Z80::ProcessINT()
{
    switch(tCycle)
    {
    case 1:
        M1 = false;
        break;
    case 3:
        IORQ = false;
        GateArray::AckInt();
        break;
    case 4:
        if (!WAIT)
        {
            tCycle--;
            return false;
        }
        break;
    case 5:
        M1 = true;
        IORQ = true;
        R = (R & 0x80) | ((R + 1) & 0x7F);
        idMode = IDMode::INTEXEC;
        return true;
    }
    return false;
}

bool Z80::ProcessFETCH()
{
    switch(tCycle)
    {
    case 1:
        M1 = false;
        MREQ = false;
        RD = false;
        DR = CPC::GetByteAt(PC);
        break;
    case 2:
        if (!WAIT)
        {
            tCycle--;
            return false;
        }
        if (halted)
            IR = 0x00;
        else
        {
            PC++;
            IR = DR;
        }
        break;
    case 3:
        R = (R & 0x80) | ((R + 1) & 0x7F);
        MREQ = true;
        RD = true;
        M1 = true;
        break;
    case 4:
        //if (!ExtendedM1[IR] || (idMode != IDMode::BASIC && idMode != IDMode::IDX))
        return true;
        //break;
        //case 8:
        //return true;
    }
    return false;
}

bool Z80::ProcessREAD()
{
    switch(tCycle)
    {
    case 1:
        MREQ = false;
        RD = false;
        return false;
    case 2:
        if (!WAIT)
            tCycle--;
        return false;
    case 3:
        DR = CPC::GetByteAt(AR);
        MREQ = true;
        RD = true;
        return true;
    }
    return false;
}

bool Z80::ProcessWRITE()
{
    switch(tCycle)
    {
    case 1:
        MREQ = false;
        return false;
    case 2:
        if (!WAIT)
            tCycle--;
        WR = false;
        CPC::SetByteAt(AR, DR);
        return false;
    case 3:
        MREQ = true;
        WR = true;
        return true;
    }
    return false;
}

bool Z80::ProcessIN()
{
    switch(tCycle)
    {
    case 1:
        break;
    case 2:
        IORQ = false;
        RD = false;
        if (!(AR & 0x4000)) DR = CRTC::RD((AR & 0x0300) >> 8);
        else if (!(AR & 0x0800)) DR = PPI::RD((AR & 0x0300) >> 8);
        else if (!(AR & 0x0480))
        {
            if ((AR & 0x0100) != 0)
            {
                if ((AR & 0x0001) == 0)
                    DR = FDC::RD_State();
                else
                    DR = FDC::RD_Data();
            }
        }
        break;
    case 3:
        if (!WAIT)
        {
            tCycle--;
            return false;
        }
        break;
    case 4:
        IORQ = true;
        RD = true;
        return true;
    }
    return false;
}

bool Z80::ProcessOUT()
{
    switch(tCycle)
    {
    case 1:
        break;
    case 2:
        IORQ = false;
        WR = false;
        if (!(AR & 0x8000)) GateArray::WR(DR);
        else if (!(AR & 0x4000)) CRTC::WR((AR & 0x0300) >> 8, DR);
        else if (!(AR & 0x2000)) CPC::SelectROM(DR);
        else if (!(AR & 0x0800)) PPI::WR((AR & 0x0300) >> 8, DR);
        else if (!(AR & 0x0480))
        {
            if ((AR & 0x0100) == 0)
                FDC::SetMotor(DR);
            else if ((AR & 0x0001) != 0)
            FDC::WR(DR);
        }

        break;
    case 3:
        if (!WAIT)
        {
            tCycle--;
            return false;
        }
        break;
    case 4:
        IORQ = true;
        WR = true;
        return true;
    }
    return false;
}

bool Z80::ProcessRELADDR()
{
    switch(tCycle)
    {
    case 4:
        AR += (sbyte)DR;
        return true;
    }
    return false;
}

bool Z80::ProcessALU4()
{
    switch(tCycle)
    {
    case 4:
        return true;
    }
    return false;
}

void Z80::Clock()
{
    RunMCycle();
}

void Z80::Clock2()
{
    if (lastTCycle)
    {
        if (lastMCycle)
        {
            mCycle = 1;
            if (!InterruptRequest && IFF1)
            {
                IFF1 = false;
                IFF2 = false;
                mCycleType = MCycleType::INT;
                if (halted)
                {
                    halted = false;
                    PC++;
                }
                EIRequest = false;
            }
            else
            {
                idMode = IDMode::BASIC;
                mCycleType = MCycleType::FETCH;
                stopPoint = true;
                if (EIRequest)
                {
                    IFF1 = true;
                    IFF2 = true;
                    EIRequest = false;
                }
            }

        }
        else
            mCycle++;
    }
}

void Z80::RunTCycle()
{
    switch(mCycleType)
    {
    case MCycleType::READ: lastTCycle = ProcessREAD(); break;
    case MCycleType::WRITE: lastTCycle = ProcessWRITE(); break;
    case MCycleType::FETCH: lastTCycle = ProcessFETCH(); break;
    case MCycleType::IN: lastTCycle = ProcessIN(); break;
    case MCycleType::OUT: lastTCycle = ProcessOUT(); break;
    case MCycleType::ALU4: lastTCycle = ProcessALU4(); break;
    case MCycleType::INT: lastTCycle = ProcessINT();  break;
    case MCycleType::RELADDR: lastTCycle = ProcessRELADDR(); break;
    }
    nops++;
    if (lastTCycle)
        tCycle = 1;
    else
        tCycle++;
}

void Z80::RunMCycle()
{
    RunTCycle();
    if (lastTCycle)
    {
        switch(idMode)
        {
        case IDMode::BASIC: lastMCycle = Step_basic(); break;
        case IDMode::MISC: lastMCycle = Step_misc(); break;
        case IDMode::BIT: lastMCycle = Step_CB(); break;
        case IDMode::IDX: lastMCycle = Step_IDX(); break;
        case IDMode::IDXBIT: lastMCycle = Step_IDX_CB(); break;
        case IDMode::INTEXEC: lastMCycle = Step_Int_Exec(); break;
        }
    }
}

void Z80::IRQ()
{
    InterruptRequest = false;
}

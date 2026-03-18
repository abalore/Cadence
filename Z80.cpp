#include "Z80.h"
#include "CPC.h"

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
bool Z80::edge;
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
    if (edge)
    {
        switch(tCycle)
        {
        case 1:
            M1 = false;
            break;
        case 2:
            IORQ = false;
            break;
        }
    }
    else
    {
        switch(tCycle)
        {
        case 5:
            t8 = CPC::DataBUS;
            M1 = true;
            IORQ = true;;
            break;
        case 6:
            return true;
        }
    }
    return false;
}

bool Z80::ProcessFETCH()
{
    if (edge)
    {
        switch(tCycle)
        {
        case 1:
            CPC::AddressBUS = PC;
            M1 = false;
            break;
        case 2:
            PC++;
            break;
        case 3:
            R = (R & 0x80) | ((R + 1) & 0x7F);
            MREQ = true;
            RD = true;
            M1 = true;
            break;
        case 4:
            break;
        }
    }
    else
    {
        switch(tCycle)
        {
        case 1:
            MREQ = false;
            RD = false;
            break;
        case 2:
            IR = CPC::DataBUS;
            break;
        case 3:
            MREQ = false;
            break;
        case 4:
            MREQ = true;
            return true;
        }
    }
    return false;

    /*

    if (halted)
        IR = 0x00;
    else
    {
        AR = PC;
        ProcessREAD();
        IR = DR;
        PC++;
    }
*/
}

bool Z80::ProcessREAD()
{
    if (edge)
        switch(tCycle)
        {
        case 1:
            CPC::AddressBUS = AR;
            break;
        case 2:
            break;
        case 3:
            DR = CPC::DataBUS;
            break;
        }
    else
        switch(tCycle)
        {
        case 1:
            MREQ = false;
            RD = false;
            break;
        case 2:
            break;
        case 3:
            MREQ = true;
            RD = true;
            return true;
        }
    return false;
}

bool Z80::ProcessWRITE()
{
    if (edge)
    {
        switch(tCycle)
        {
        case 1:
            CPC::AddressBUS = AR;
            break;
        }
    }
    else
    {
        switch(tCycle)
        {
        case 1:
            MREQ = false;
            CPC::DataBUS = DR;
            break;
        case 2:
            WR = false;
            break;
        case 3:
            MREQ = true;
            WR = true;
            return true;
        }
    }
    return false;
}

bool Z80::ProcessIN()
{
    if (edge)
    {
        switch(tCycle)
        {
        case 1:
            CPC::AddressBUS = AR;
            break;
        case 2:
            IORQ = false;
            RD = false;
            break;
        case 3:
            break;
        case 4:
            DR = CPC::DataBUS;
            break;
        }
    }
    else
    {
        switch(tCycle)
        {
        case 1:
            break;
        case 2:
            break;
        case 3:
            break;
        case 4:
            IORQ = true;
            RD = true;
            return true;
        }
    }
    return false;
}

bool Z80::ProcessOUT()
{
    if (edge)
    {
        switch(tCycle)
        {
        case 1:
            CPC::AddressBUS = AR;
            break;
        case 2:
            IORQ = false;
            WR = false;
            break;
        case 3:
            break;
        case 4:
            break;
        }
    }
    else
    {
        switch(tCycle)
        {
        case 1:
            CPC::DataBUS = DR;
            break;
        case 2:
            break;
        case 3:
            break;
        case 4:
            IORQ = true;
            WR = true;
            return true;
        }
    }
    return false;
}

bool Z80::ProcessRELADDR()
{
    if (!edge)
        switch(tCycle)
        {
        case 5:
            AR += (sbyte)DR;
            return true;
        }
    return false;
}

bool Z80::ProcessALU1()
{
    if (!edge)
        switch(tCycle)
        {
        case 1:
            return true;
        }
    return false;
}

bool Z80::ProcessALU2()
{
    if (!edge)
        switch(tCycle)
        {
        case 3:
            return true;
        }
    return false;
}

bool Z80::ProcessALU3()
{
    if (!edge)
        switch(tCycle)
        {
        case 3:
            return true;
        }
    return false;
}

bool Z80::ProcessALU4()
{
    if (!edge)
        switch(tCycle)
        {
        case 4:
            return true;
        }
    return false;
}

void Z80::Clock()
{
    edge = !edge;
    RunMCycle();
}

void Z80::Clock2()
{
    if (!edge && lastTCycle)
    {
        if (lastMCycle)
        {
            mCycle = 1;
            if (!InterruptRequest && IFF1)
            {
                IFF1 = false;
                IFF2 = false;
                mCycleType = MCycleType::INT;
                idMode = IDMode::INTEXEC;
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
    lastTCycle = false;
    switch(mCycleType)
    {
    case MCycleType::READ: lastTCycle = ProcessREAD(); break;
    case MCycleType::WRITE: lastTCycle = ProcessWRITE(); break;
    case MCycleType::FETCH: lastTCycle = ProcessFETCH(); break;
    case MCycleType::IN: lastTCycle = ProcessIN(); break;
    case MCycleType::OUT: lastTCycle = ProcessOUT(); break;
    case MCycleType::ALU1: lastTCycle = ProcessALU1(); break;
    case MCycleType::ALU2: lastTCycle = ProcessALU2(); break;
    case MCycleType::ALU3: lastTCycle = ProcessALU3(); break;
    case MCycleType::ALU4: lastTCycle = ProcessALU4(); break;
    case MCycleType::INT: lastTCycle = ProcessINT();  break;
    case MCycleType::RELADDR: lastTCycle = ProcessRELADDR(); break;
    }
    if (!edge)
    {
        nops++;
        if (lastTCycle)
        {
            tCycle = 1;
        }
        else
            tCycle++;
    }
}

void Z80::RunMCycle()
{
    lastMCycle = false;
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

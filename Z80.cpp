#include "Z80.h"
#include "CPC.h"

Z80::Z80()
    : WAIT(false), stopPoint(false), InterruptRequest(true),
      tCycle(1), mCycle(1), MREQ(true), IORQ(true), RD(true), WR(true),
      PC(0), SP(0),
      A(0), F(0), B(0), C(0), D(0), E(0), H(0), L(0),
      IXH(0), IXL(0), IYH(0), IYL(0),
      A_(0), F_(0), B_(0), C_(0), D_(0), E_(0), H_(0), L_(0),
      t16H(0), t16L(0), SPH(0), SPL(0),
      AF(&A, &F), BC(&B, &C), DE(&D, &E), HL(&H, &L),
      IX(&IXH, &IXL), IY(&IYH, &IYL),
      AF_(&A_, &F_), BC_(&B_, &C_), DE_(&D_, &E_), HL_(&H_, &L_),
      fS(false), fZ(false), fH(false), fP(false), fN(false), fC(false), f3(false), f5(false),
      I(0), R(0), IR(0), DR(0), AR(0),
      idMode(IDMode::BASIC), mCycleType(MCycleType::FETCH),
      IFF1(false), IFF2(false), EIRequest(false), halted(false), im(0), nops(0), M1(false),
      t8(0),
      t16(&t16H, &t16L), tt16(&t16H, &t16L),
      index(0), tByte(0), opCode(0),
      IDX(&IX),
      t_cp(0), tC(false), tCV(0), t(0),
      w1(0), w2(0), w3(0), i1(0), i2(0), i3(0), s1(0),
      intACK(0), lastMCycle(false), lastTCycle(false)
{}

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
    tCycle = 1;
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
    M1 = false;
    AR = 0;
    IR = 0;
    DR = 0;
    t8 = 0;
    opCode = 0;
}

bool Z80::ProcessINT()
{
    switch(tCycle)
    {
    case 1:
        M1 = false;
        IORQ = false;
        break;
    case 2:
        CPC::AckInt();
        break;
    case 3:
        break;
    case 4:
        if (!WAIT)
            tCycle--;
        break;
    case 7:
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
        if ((ExtendedM1[IR] == 1 || ExtendedM1[IR] == 2) && (idMode == IDMode::BASIC || idMode == IDMode::IDX))
            return false;
        if (ExtendedM1[IR] == 3 && idMode == IDMode::MISC)
            return false;
        return true;
    case 5:
        if (ExtendedM1[IR] == 3 && idMode == IDMode::MISC)
            return false;
        return true;
    case 6:
        return true;
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

bool Z80::ProcessREAD4()
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
        return false;
    case 4:
        return true;
    }
    return false;
}

bool Z80::ProcessREAD5()
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
        return false;
    case 5:
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

bool Z80::ProcessWRITE5()
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
        return false;
    case 5:
        return true;
    }
    return false;
}

bool Z80::ProcessWRITE4()
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
        return false;
    case 4:
        return true;
    }
    return false;
}

bool Z80::ProcessWRITEI()
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
        return false;
    case 4:
        DE.Set(DE.Get() + 1);
        HL.Set(HL.Get() + 1);
        return false;
    case 5:
        BC.Set(BC.Get() - 1);
        return true;
    }
    return false;
}

bool Z80::ProcessWRITED()
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
        return false;
    case 4:
        DE.Set(DE.Get() - 1);
        HL.Set(HL.Get() - 1);
        return false;
    case 5:
        BC.Set(BC.Get() - 1);
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
        break;
    case 3:
        if (!WAIT)
        {
            tCycle--;
            return false;
        }
        break;
    case 4:
        DR = CPC::PortRead(AR);
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
        CPC::PortWrite(AR, DR);
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

bool Z80::ProcessALU(BYTE length)
{
    return tCycle == length;
}

void Z80::Clock()
{
    Z80::stopPoint = false;
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
    case MCycleType::ALU3: lastTCycle = ProcessALU(3); break;
    case MCycleType::ALU4: lastTCycle = ProcessALU(4); break;
    case MCycleType::ALU5: lastTCycle = ProcessALU(5); break;
    case MCycleType::INT: lastTCycle = ProcessINT();  break;
    case MCycleType::RELADDR: lastTCycle = ProcessRELADDR(); break;
    case MCycleType::WRITEI: lastTCycle = ProcessWRITEI(); break;
    case MCycleType::WRITED: lastTCycle = ProcessWRITED(); break;
    case MCycleType::WRITE5: lastTCycle = ProcessWRITE5(); break;
    case MCycleType::READ4: lastTCycle = ProcessREAD4(); break;
    case MCycleType::WRITE4: lastTCycle = ProcessWRITE4(); break;
    case MCycleType::READ5: lastTCycle = ProcessREAD5(); break;
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

Z80DebugState Z80::GetDebugState()
{
    EncodeF();
    Z80DebugState s;
    s.AF = AF.Get(); s.BC = BC.Get(); s.DE = DE.Get(); s.HL = HL.Get();
    s.IX = IX.Get(); s.IY = IY.Get();
    s.PC = PC; s.SP = SP;
    s.fS = fS; s.fZ = fZ; s.f5 = f5; s.fH = fH;
    s.f3 = f3; s.fP = fP; s.fN = fN; s.fC = fC;
    s.InterruptRequest = InterruptRequest;
    s.IFF1 = IFF1; s.IFF2 = IFF2;
    s.R = R; s.I = I; s.im = im;
    s.nops = nops;
    return s;
}


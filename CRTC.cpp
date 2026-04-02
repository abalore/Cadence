#include "CRTC.h"
#include "CPC.h"
#include "Z80.h"
#include "GateArray.h"

BYTE CRTC::Index;
BYTE CRTC::RA;
bool CRTC::HSYNC;
bool CRTC::VSYNC;
bool CRTC::BORDER;

BYTE CRTC::HCC;
word CRTC::VLC;
BYTE CRTC::VCC;
BYTE CRTC::VSC;
BYTE CRTC::HSC;
word CRTC::MA;

BYTE CRTC::HT;
BYTE CRTC::HD;
BYTE CRTC::HSP;
BYTE CRTC::HSW;
BYTE CRTC::VSW;
BYTE CRTC::VT;
BYTE CRTC::VTA;
BYTE CRTC::VD;
BYTE CRTC::VSP;
BYTE CRTC::IS;
BYTE CRTC::MRA;

BYTE CRTC::latchMRA;

BYTE CRTC::VTAC;
BYTE CRTC::verticalTotal;
bool CRTC::HDISP;
bool CRTC::VDISP;
word CRTC::baseMA;
bool CRTC::adjustMode;

BYTE CRTC::crtcType = 0;

word CRTC::DSA;

void CRTC::Reset()
{
    Index = 0;
    RA = 0;
    MA = 0;
    HCC = 0;
    VCC = 0;
    HSC = 0;
    VSC = 0;
    VLC = 0;
    HSYNC = false;
    VSYNC = false;
    HDISP = true;
    VDISP = true;
    BORDER = false;
    HT = 63;
    HD = 40;
    HSP = 46;
    VSW = 8;
    HSW = 14;
    VT = 38;
    VTA = 0;
    VD = 25;
    VSP = 30;
    MRA = 7;
    DSA = 0x3000;
    VTAC = 0;
    latchMRA = 0;
    adjustMode = false;
    DSA = 0;
    MA = DSA;
    VSW = 8; //CPC::DataBUS >> 4;
}

BYTE CRTC::RD(BYTE address)
{
    switch(address)
    {
    case 2: // Status out
        return 0;
    case 3: // Data out
        switch(Index)
        {
        case 0:
            return HT;
        case 1:
            return HD;
        case 2:
            return HSP;
        case 3:
            return VSW * 16 + HSW;
        case 4:
            return VT;
        case 5:
            return VTA;
        case 6:
            return VD;
        case 7:
            return VSP;
        case 8:
            return IS;
        case 9:
            return MRA;
        case 12:
            return DSA >> 8;
        case 13:
            return DSA & 0xFF;
        }
        break;
    }
    return 0;
}

void CRTC::WR(BYTE address, BYTE value)
{
    switch(address)
    {
    case 0: // Index in
        Index = value;
        break;
    case 1: // Data in
        switch(Index)
        {
        case 0:
            HT = value;
            break;
        case 1:
            HD = value;
            break;
        case 2:
            HSP = value;
            break;
        case 3:
            HSW = value & 0x0F;
            VSW = value >> 4;
            if (VSW == 0)
                VSW = 16;               // Check CRTC Type
            break;
        case 4:
            VT = value & 0x7F;
            break;
        case 5:
            VTA = value & 0x1F;
            break;
        case 6:
            VD = value & 0x7F;
            break;
        case 7:
            VSP = value & 0x7F;
            break;
        case 8:
            IS = value;
            break;
        case 9:
            MRA = value;
            break;
        case 12:
            DSA &= 0x00FF;
            DSA |= (value & 0x3F) * 256;
            break;
        case 13:
            DSA &= 0xFF00;
            DSA |= value;
            break;
        }

        break;
    }
}

void CRTC::RunCombinational()
{
}

void CRTC::RunHorizontalChar()
{
    MA = (MA + 1) & 0x3FFF;
    if (HCC == HT)
    {
        MA = baseMA;
        HCC = 0;
        if (HD > 0)
            HDISP = true;
        RunLine();
    }
    else
    {
        HCC++;
    }
    if (HCC == HSP)
    {
        HSYNC = true;
        HSC = 0;
    }
    if (HSYNC)
    {
        if (HSC == HSW)
            HSYNC = false;
        else HSC++;
    }
    if (HCC == HD)
    {
        HDISP = false;
        if (RA == MRA)
            baseMA = MA;
    }
}

void CRTC::RunLine()
{
    if (VSYNC)
    {
        if (VSC == VSW) VSYNC = false; else VSC++;
    }
    if (adjustMode)
    {
        if (RA == latchMRA)
        {
            baseMA = DSA;
            MA = baseMA;
            VDISP = (VD > 0);
            VCC = 0;
            adjustMode = false;
            RA = 0;
            if (VCC == VSP)
            {
                VSYNC = true;
                VSC = 0;
            }
            if (VCC == VD) VDISP = false;
        }
        else
            RA = (RA + 1) & 0x1F;
    }
    else
    {
        if (RA == MRA)
        {
            RA = 0;
            VCC = (VCC + 1) & 0x7F;
            if (VCC == VT) adjustMode = true;
            if (VCC == VSP)
            {
                VSYNC = true;
                VSC = 0;
            }
            if (VCC == VD) VDISP = false;
        }
        else
            RA = (RA + 1) & 0x1F;
    }
    latchMRA = MRA + VTA;
}

void CRTC::RunVerticalChar()
{

}

void CRTC::ResetFrame()
{

}

void CRTC::Clock()
{
    RunHorizontalChar();
    BORDER = !HDISP || !VDISP;
}

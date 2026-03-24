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
    HDISP = false;
    HDISP = false;
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
    adjustMode = false;
    DSA = 0;
    MA = DSA;
    VSW = 8; //CPC::DataBUS >> 4;
}

void CRTC::RD()
{
    switch((CPC::AddressBUS & 0x0300) >> 8)
    {
    case 2: // Status out
        //////////////////////////////////////////
        break;
    case 3: // Data out
        switch(Index)
        {
        case 0:
            CPC::DataBUS = HT;
            break;
        case 1:
            CPC::DataBUS = HD;
            break;
        case 2:
            CPC::DataBUS = HSP;
            break;
        case 3:
            HSW = CPC::DataBUS = VSW * 16 + HSW;
            break;
        case 4:
            CPC::DataBUS = VT;
            break;
        case 5:
            CPC::DataBUS = VTA;
            break;
        case 6:
            CPC::DataBUS = VD;
            break;
        case 7:
            CPC::DataBUS = VSP;
            break;
        case 8:
            CPC::DataBUS = IS;
            break;
        case 9:
            MRA = CPC::DataBUS = MRA;
            break;
        case 12:
            CPC::DataBUS = DSA >> 8;
            break;
        case 13:
            CPC::DataBUS = DSA & 0xFF;
            break;
        }

        break;
    }
}

void CRTC::WR()
{
    switch((CPC::AddressBUS & 0x0300) >> 8)
    {
    case 0: // Index in
        Index = CPC::DataBUS;
        break;
    case 1: // Data in
        switch(Index)
        {
        case 0:
            HT = CPC::DataBUS;
            break;
        case 1:
            HD = CPC::DataBUS;
            break;
        case 2:
            HSP = CPC::DataBUS;
            break;
        case 3:
            HSW = CPC::DataBUS & 0x0F;
            VSW = CPC::DataBUS >> 4;
            if (VSW == 0)
                VSW = 16;               // Check CRTC Type
            break;
        case 4:
            VT = CPC::DataBUS & 0x7F;
            break;
        case 5:
            VTA = CPC::DataBUS & 0x1F;
            break;
        case 6:
            VD = CPC::DataBUS & 0x7F;
            break;
        case 7:
            VSP = CPC::DataBUS & 0x7F;
            break;
        case 8:
            IS = CPC::DataBUS;
            break;
        case 9:
            MRA = CPC::DataBUS;
            break;
        case 12:
            DSA &= 0x00FF;
            DSA |= (CPC::DataBUS & 0x3F) * 256;
            break;
        case 13:
            DSA &= 0xFF00;
            DSA |= CPC::DataBUS;
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
    if (HDISP && VDISP)
        MA++;
    if (HCC == HT)
    {
        HCC = 0;
        if (HD > 0)
            HDISP = true;
        RunLine();
    }
    else
    {
        HCC++;
    }
    if (HSYNC)
    {
        HSC++;
        if (HSC == HSW || (HSW == 0 && HSC == 16))
            HSYNC = false;
    }
    if (HCC == HSP - 1)
    {
        HSYNC = true;
        HSC = 0;
    }
    if (HCC == HD)
        HDISP = false;
}

void CRTC::RunLine()
{
    if (VSYNC)
    {
        VSC++;
        if (VSC == VSW || (VSW == 0 && VSC == 16))
            VSYNC = false;
    }

    if (adjustMode ? (RA == VTA - 1) : (RA == MRA))
    {
        RA = 0;

        if (adjustMode)
        {
            baseMA = DSA;
            VCC = 0;
            adjustMode = false;

            if (VSP == 0)
            {
                VSYNC = true;
                VSC = 0;
            }
            VDISP = (VD > 0);
        }
        else
        {
            baseMA = MA;
            RunVerticalChar();
        }
    }
    else
    {
        RA = (RA + 1) & 0x1F;
    }
    MA = baseMA;
}

void CRTC::RunVerticalChar()
{
    if (VCC == VT)
    {
        if (VTA == 0)
        {
            baseMA = DSA;
            VDISP = (VD > 0);
            VCC = 0;
        }
        else
        {
            adjustMode = true;
        }
    } else
    {
        VCC = (VCC + 1) & 0x7F;
    }
    if (VCC == VD)
        VDISP = false;
    if (VCC == VSP)
    {
        VSYNC = true;
        VSC = 0;
    }
}

void CRTC::ResetFrame()
{
}

void CRTC::Clock()
{
    RunHorizontalChar();
    BORDER = !HDISP || !VDISP;
}

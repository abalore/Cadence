#include "Headers/CRTC.h"
#include "Headers/CPC.h"
#include "Headers/Z80.h"
#include "Headers/GateArray.h"

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
    VSW = 8; //Z80::DR >> 4;
}

void CRTC::RD()
{
    switch((Z80::AR & 0x0300) >> 8)
    {
    case 2: // Status out
        //////////////////////////////////////////
        break;
    case 3: // Data out
        switch(Index)
        {
        case 0:
            Z80::DR = HT;
            break;
        case 1:
            Z80::DR = HD;
            break;
        case 2:
            Z80::DR = HSP;
            break;
        case 3:
            HSW = Z80::DR = VSW * 16 + HSW;
            break;
        case 4:
            Z80::DR = VT;
            break;
        case 5:
            Z80::DR = VTA;
            break;
        case 6:
            Z80::DR = VD;
            break;
        case 7:
            Z80::DR = VSP;
            break;
        case 8:
            Z80::DR = IS;
            break;
        case 9:
            MRA = Z80::DR = MRA;
            break;
        case 12:
            Z80::DR = DSA >> 8;
            break;
        case 13:
            Z80::DR = DSA & 0xFF;
            break;
        }

        break;
    }
}

void CRTC::WR()
{
    switch((Z80::AR & 0x0300) >> 8)
    {
    case 0: // Index in
        Index = Z80::DR;
        break;
    case 1: // Data in
        switch(Index)
        {
        case 0:
            HT = Z80::DR;
            break;
        case 1:
            HD = Z80::DR;
            break;
        case 2:
            HSP = Z80::DR;
            break;
        case 3:
            HSW = Z80::DR & 0x0F;
            VSW = Z80::DR >> 4;
            if (VSW == 0)
                VSW = 16;               // Check CRTC Type
            break;
        case 4:
            VT = Z80::DR & 0x7F;
            break;
        case 5:
            VTA = Z80::DR & 0x1F;
            break;
        case 6:
            VD = Z80::DR & 0x7F;
            break;
        case 7:
            VSP = Z80::DR & 0x7F;
            break;
        case 8:
            IS = Z80::DR;
            break;
        case 9:
            MRA = Z80::DR;
            break;
        case 12:
            DSA &= 0x00FF;
            DSA |= (Z80::DR & 0x3F) * 256;
            break;
        case 13:
            DSA &= 0xFF00;
            DSA |= Z80::DR;
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
    if (HSYNC)
    {
        HSC++;
        if (HSC == HSW)
            HSYNC = false;
    }
    if (HDISP && VDISP)
        MA++;
    if (HCC == HT)
    {
        HCC = 0;
        HDISP = true;
        RunLine();
    } else HCC++;
    if (HCC == HD)
        HDISP = false;
    if (HCC == HSP)
    {
        HSYNC = true;
        HSC = 0;
    }
}

void CRTC::RunLine()
{
    if (VSYNC)
    {
        VSC++;
        VSC&=0x0F;
        if (VSC == VSW || VSC == 0)
            VSYNC = false;
    }
    if (adjustMode)
    {
        RA++;
        RA&=0x1F;
        if (RA == VTA)
        {
            VDISP = true;
            MA = DSA;
            baseMA = MA;
            adjustMode = false;
            RA = 0;
        }
        else
        {
            MA = baseMA;
        }
    }
    else
        if (RA == MRA)
        {
            RA = 0;
            baseMA = MA;
            RunVerticalChar();
        }
        else
        {
            RA++;
            RA&=0x1F;
            MA = baseMA;
        }
}

void CRTC::RunVerticalChar()
{
    if (VCC == VT)
    {
        VCC = 0;
        if (VTA == 0)
        {
            VDISP = true;
            MA = DSA;
            baseMA = MA;
        }
        else
            adjustMode = true;
    } else
    {
        VCC++;
        VCC&=0x7F;
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
    BORDER = !HDISP || !VDISP;// || verticalAdjust > 0;
}

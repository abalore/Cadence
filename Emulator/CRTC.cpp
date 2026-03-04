#include "Headers/CRTC.h"
#include "Headers/CPC.h"
#include "Headers/Z80.h"
#include "Headers/GateArray.h"

BYTE CRTC::R[18];
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

BYTE CRTC::R0;
BYTE CRTC::R1;
BYTE CRTC::R2;
BYTE CRTC::R3;
BYTE CRTC::R4;
BYTE CRTC::R5;
BYTE CRTC::R6;
BYTE CRTC::R7;
BYTE CRTC::R8;
BYTE CRTC::R9;
BYTE CRTC::R10;
BYTE CRTC::R11;
BYTE CRTC::R12;
BYTE CRTC::R13;
BYTE CRTC::R14;
BYTE CRTC::R15;
BYTE CRTC::R16;
BYTE CRTC::R17;
BYTE CRTC::VTAC;
BYTE CRTC::verticalTotal;
bool CRTC::HDISP;
bool CRTC::VDISP;
word CRTC::baseMA;
bool CRTC::adjustMode;

BYTE CRTC::crtcType = 0;

void CRTC::Reset()
{
    for (int i = 0; i < 18; i++)
        R[i] = 0;
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
    R0 = 0;
    R1 = 0;
    R2 = 0;
    R3 = 0;
    R4 = 0;
    R5 = 0;
    R6 = 0;
    R7 = 0;
    R8 = 0;
    R9 = 0;
    R10 = 0;
    R11 = 0;
    R12 = 0;
    R13 = 0;
    R14 = 0;
    R15 = 0;
    R16 = 0;
    R17 = 0;
    VTAC = 0;
    adjustMode = false;
}

void CRTC::RD()
{
    switch((Z80::AR & 0x0300) >> 8)
    {
    case 2: // Status out
        //////////////////////////////////////////
        break;
    case 3: // Data out
        Z80::DR = R[Index];
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
        R[Index] = Z80::DR;
        break;
    }
}

void CRTC::RunHorizontalChar()
{
    if (!BORDER) MA = (MA + 1) & 0x3FFF;

    if (HCC == R[0])
    {
        HCC = 0;
        HDISP = true;
        RunLine();
    }
    else
        HCC++;

    if (HCC == (R[2]))
    {
        HSYNC = true;
        HSC = R[3] & 0x0F;
    }
    if (HSC > 0)
    {
        HSC--;
        if (HSC == 0)
            HSYNC = false;
    }
    if (HCC == R[1])
        HDISP = false;
}

void CRTC::RunLine()
{
    if (adjustMode && RA == VTAC - 1)
        ResetFrame();
    else if (RA == R[9])
    {
        RA = 0;
        RunVerticalChar();
        baseMA = MA;
    }
    else
        RA++;
    MA = baseMA;
    if (VSC > 0)
    {
        VSC--;
        if (VSC == 0)
            VSYNC = false;
    }
}

void CRTC::RunVerticalChar()
{
    if (VCC == R[4])
    {
        if (!adjustMode)
        {
            VTAC = R[5];
            if (VTAC > 0)
                adjustMode = true;
            else
                ResetFrame();
        }
        else
            ResetFrame();
        return;
    }
    else
        VCC++;
    if (VCC == R[7])
    {
        VSYNC = true;
        VSC = R[3] >> 4;
        if (VSC == 0)
            VSC = 16;
    }
    if (VCC == R[6])
        VDISP = false;
}

void CRTC::ResetFrame()
{
    MA = ((R[12] & 0x3F) << 8) + R[13];
    VDISP = true;
    HCC = 0;
    VCC = 0;
    //HSC = 0;
    //VSC = 0;
    adjustMode = false;
    baseMA = MA;
    RA = 0;
}

void CRTC::Clock()
{
    RunHorizontalChar();
    BORDER = !HDISP || !VDISP;// || verticalAdjust > 0;
}

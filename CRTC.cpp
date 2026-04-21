#include "CRTC.h"

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
    adjustMode = false;
    MA = DSA;
    baseMA = DSA;
    VSW = 8;

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
            //if (VSW == 0)
//                VSW = 16;               // Check CRTC Type
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

void CRTC::RunHorizontalChar()
{
    MA = (MA + 1) & 0x3FFF;
    // Timing for CRTC 0
    if (HCC == 1)
        EndScreen = VT == VCC;

    if (HCC == 2)
    {
        if (RA == MRA && VCC == VT && VTA > 0)
            willAdjust = true;
    }

    if (HCC == HT)
    {
        MA = baseMA;
        HCC = 0;
        if (HD > 0)
            HDISP = true;
        if (!EndScreen)
            EndChar = RA == MRA;
        EndOfLine();

        EndChar = RA == MRA;
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
        else HSC = (HSC + 1) & 0x0F;
    }
    if (HCC == HD)
    {
        HDISP = false;
        if (RA == MRA)
            baseMA = MA;
    }
    if (!VSYNC && VCC == VSP)
    {
        VSYNC = true;
        VSC = 0;
    }

    if (VDISP && VCC == VD) VDISP = false;
}

void CRTC::EndOfLine()
{
    if (VSYNC)
    {
        VSC = (VSC + 1) & 0x0F;
        if (VSC == VSW)
        {
            VSYNC = false;
            VSC = 0;
        }
    }

    bool resetFrame = false;
    if (adjustMode)
    {
        RA = (RA + 1) & 0x1F;
        if (RA == VTA)
        {
            RA = 0;
            adjustMode = false;
            resetFrame = true;
            willAdjust = false;
        }
    }
    else
    {
        if (EndChar)
        {
            RA = 0;
            if (EndScreen)
            {
                if (willAdjust) adjustMode = true;
                else
                    resetFrame = true;
            }
            else
            {
                VCC = (VCC + 1) & 0x7F;
            }
        }
        else
        {
            RA = (RA + 1) & 0x1F;
        }
    }
    if (resetFrame)
    {
        baseMA = DSA;
        MA = baseMA;
        VDISP = (VD > 0);
        VCC = 0;
    }

}

void CRTC::Clock()
{
    RunHorizontalChar();
    BORDER = !HDISP || !VDISP;
}

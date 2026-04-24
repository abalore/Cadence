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
    VLC = 0;
    vccWrapped = false;
    vsyncFiredThisFrame = false;

}

BYTE CRTC::RD(BYTE address)
{
    if (crtcType == 3 || crtcType == 4)
    {
        // Types 3/4: &BE00 and &BF00 both use the 3 LSB of the index
        switch (Index & 7)
        {
        case 0: return 0;                              // R16 (LPEN H)
        case 1: return 0;                              // R17 (LPEN L)
        case 2: return 0;                              // R10 (ASIC status 1)
        case 3: return (crtcType == 4) ? 0x08 : 0x00;  // R11 (ASIC status 2): bit 3 distinguishes type 4 from type 3
        case 4: return DSA >> 8;                       // R12
        case 5: return DSA & 0xFF;                     // R13
        case 6: return CH;                             // R14
        case 7: return CL;                             // R15
        }
        return 0;
    }

    switch (address)
    {
    case 2: // Status port &BE
        if (crtcType == 1)
        {
            BYTE s = 0;
            if (!VDISP) s |= 0x20; // bit 5: vertical border (past R6)
            return s;
        }
        // Types 0, 2: high-Z → floating bus
        return 0xFF;
    case 3: // Data port &BF
        switch (crtcType)
        {
        case 0: // HD6845S: R12-R17 readable, 5-bit index truncation
            switch (Index & 0x1F)
            {
            case 12: return DSA >> 8;
            case 13: return DSA & 0xFF;
            case 14: return CH;
            case 15: return CL;
            case 16: return 0;
            case 17: return 0;
            }
            return 0;
        case 1: // UM6845R: R14-R17 readable; indices with lower-5-bits all 1 return 0xFF
            if ((Index & 0x1F) == 0x1F) return 0xFF;
            switch (Index & 0x1F)
            {
            case 14: return CH;
            case 15: return CL;
            case 16: return 0;
            case 17: return 0;
            }
            return 0;
        case 2: // MC6845: only R16/R17 readable
            switch (Index & 0x1F)
            {
            case 16: return 0;
            case 17: return 0;
            }
            return 0;
        }
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
        case 14:
            CH = value & 0x3F;
            break;
        case 15:
            CL = value;
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
        HSC = 0;
        if (HSW == 0)
            HSYNC = !(crtcType == 0 || crtcType == 1); // types 2/3/4: start a 16-cycle wrap pulse
        else
            HSYNC = true;
    }
    if (HSYNC)
    {
        if (HSW > 0 && HSC == HSW)
            HSYNC = false;
        else
        {
            HSC = (HSC + 1) & 0x0F;
            if (HSW == 0 && HSC == 0) HSYNC = false; // wrapped past 16
        }
    }
    if (HCC == HD)
    {
        HDISP = false;
        if (RA == MRA)
            baseMA = MA;
    }
    if (!vsyncFiredThisFrame && VCC == VSP)
    {
        if (crtcType == 3 || crtcType == 4)
        {
            // Types 3/4: VSYNC only latches at the start of a character row
            if (HCC == 0 && RA == 0)
            {
                VSYNC = true;
                VSC = 0;
                vsyncFiredThisFrame = true;
            }
        }
        else
        {
            VSYNC = true;
            // Types 1/2 triggered mid-line start VSC at 1 (shortens pulse by 1 line); Type 0 always 0.
            VSC = (HCC > 0 && (crtcType == 1 || crtcType == 2)) ? 1 : 0;
            vsyncFiredThisFrame = true;
        }
    }

    if (VDISP && VCC == VD) VDISP = false;
}

void CRTC::EndOfLine()
{
    if (VSYNC)
    {
        VSC = (VSC + 1) & 0x0F;
        // Types 1 (UM6845R) and 2 (MC6845) ignore R3 high nibble; 4-bit VSC wrap gives a 16-line pulse.
        BYTE effVSW = (crtcType == 1 || crtcType == 2) ? 0 : VSW;
        if (VSC == effVSW)
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
                BYTE prev = VCC;
                VCC = (VCC + 1) & 0x7F;
                if (prev == 0x7F) vccWrapped = true;
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
        VLC = 0;
        vccWrapped = false;
        vsyncFiredThisFrame = false;
        // Types 3/4: frame reset cancels any in-progress VSYNC pulse.
        if (crtcType == 3 || crtcType == 4)
        {
            VSYNC = false;
            VSC = 0;
        }
    }
    else
    {
        VLC++;
    }

}

void CRTC::Clock()
{
    RunHorizontalChar();
    // R8 bits 7-6 = 11 forces display off on Types 0/3/4; Types 1/2 use these bits for interlace only.
    bool skewOff = ((IS & 0xC0) == 0xC0) && (crtcType == 0 || crtcType == 3 || crtcType == 4);
    BORDER = !HDISP || !VDISP || skewOff;
}

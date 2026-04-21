#include "GateArray.h"
#include "CPC.h"
#include <cstring>

void GateArray::Reset()
{
    Color = AbsoluteBlack;
    memset(INK, 0, sizeof(INK));
    Monochrome = false;
    BORDER = 0;
    currentPen = 0;
    R52 = 0;
    mode  = 0;
    nextMode = 0;
    pi = 0;
    hsyncTrigger = false;
    vsyncTrigger = false;
    LoROMActive = false;
    HiROMActive = false;
    // private
    RMR = 0;
    MMR = 0;
    borderSelected = false;
    CCLK = false;
    VideoAccess = false;
    videoAddress = 0;
    currentWord = 0;
    pixelIndex = 0;
    lastHSYNC = false;
    lastVSYNC = false;
    lastHDISP = false;
    hsyncDelay = 0;
    vsyncDelay = 0;
    blankColor = nullptr;
    currentPalette = Palette;
    latchLo = 0;
    latchHi = 0;
    dispEnFF1 = false;
    dispEnFF2 = false;
    intTimeout = 0;
    porch = 0;
    ready = 0;
    for (BYTE m = 0; m < 4; m++)
        for (BYTE i = 0; i < 8; i++)
            for (int b = 0; b < 256; b++)
                decodedPen[m][i][b] = GetPenForPixel(m, b, i);
}

void GateArray::AckInt()
{
    R52 &= 0x1F;
    CPC::z80.InterruptRequest = true;
}

void GateArray::ProcessSync()
{
    dispEnFF2 = dispEnFF1;
    dispEnFF1 = CPC::crtc.BORDER;
    bool hsyncFallingEdge = lastHSYNC && !CPC::crtc.HSYNC;
    bool hsyncRisingEdge = !lastHSYNC && CPC::crtc.HSYNC;
    bool hdispRisingEdge = !lastHDISP && CPC::crtc.HDISP;
    if (hdispRisingEdge)
    {
        mode = nextMode;
    }
    if (hsyncRisingEdge)
    {
        hsyncTrigger = true;
    }
    if (hsyncFallingEdge)
    {
        if (porch > 0) porch--;
        R52++;
        if (R52 == 52)
        {
            R52 = 0;
            CPC::z80.IRQ();
        }
        if (vsyncDelay > 0)
        {
            vsyncDelay--;
            if (vsyncDelay == 0)
            {
                vsyncTrigger = true;
                if (R52 >= 32) CPC::z80.IRQ();
                R52 = 0;
            }
        }
    }
    if (!lastVSYNC && CPC::crtc.VSYNC)
    {
        vsyncDelay = 2;
        porch = 26;
    }

    lastVSYNC = CPC::crtc.VSYNC;
    lastHSYNC = CPC::crtc.HSYNC;
    lastHDISP = CPC::crtc.HDISP;

    if (CPC::crtc.HSYNC || CPC::crtc.VSYNC)
        blankColor = AbsoluteBlack;
    else if (porch)
        blankColor = NormalBlack;
    else
        blankColor = nullptr;
}

void GateArray::SetPixel()
{
    if (blankColor)
    {
        Color = blankColor;
        return;
    }
    BYTE currentByte = (pixelIndex & 8) ? (currentWord >> 8) : (currentWord & 0xFF);
    BYTE currentInk = dispEnFF2 ? BORDER : INK[decodedPen[mode][pixelIndex & 7][currentByte]];
    if (++pixelIndex == 16) pixelIndex = 0;
    Color = &currentPalette[currentInk * 3];
}

void GateArray::SetMonochrome(bool m)
{
    Monochrome = m;
    currentPalette = m ? GreenPalette : Palette;
}

GateArrayDebugState GateArray::GetDebugState() const
{
    GateArrayDebugState s;
    s.currentPen = currentPen;
    s.BORDER = BORDER;
    s.mode = mode;
    s.R52 = R52;
    s.videoAddress = videoAddress;
    memcpy(s.INK, INK, sizeof(INK));
    s.LoROMActive = LoROMActive;
    s.HiROMActive = HiROMActive;
    return s;
}

const BYTE *GateArray::GetPaletteEntry(BYTE entry)
{
    return &Palette[INK[entry] * 3];
}

BYTE GateArray::GetPenForPixel(BYTE m, BYTE b, BYTE i)
{
    switch(m)
    {
    case 0:
        pi = i >> 2;
        return ((b & (0x80 >> pi)) > 0) +
               ((b & (0x20 >> pi)) > 0) * 4 +
               ((b & (0x08 >> pi)) > 0) * 2 +
               ((b & (0x02 >> pi)) > 0) * 8;
    case 1:
        pi = i >> 1;
        return ((b & (0x80 >> pi)) > 0) +
               ((b & (0x08 >> pi)) > 0) * 2;
    case 2:
        pi = i;
        return (b & (0x80 >> pi )) > 0;
    default:
        pi = i >> 2;
        return ((b & (0x08 >> pi)) >> (2 - pi))
               + ((b & (0x02 >> pi)) >> (1 - pi));
    }
}

void GateArray::LoadVideoAddress()
{
    videoAddress = (CPC::crtc.MA & 0x03FF) << 1;
    videoAddress += (CPC::crtc.RA & 0x07) << 11;
    videoAddress += (CPC::crtc.MA & 0x3000) << 2;

    currentWord = latchHi * 256 + latchLo;
}

void GateArray::ReadByte(bool lo)
{
    if (lo)
        latchLo = CPC::VRAMByte(videoAddress);
    else
        latchHi = CPC::VRAMByte(videoAddress);
    videoAddress++;
}

void GateArray::WR(BYTE value)
{
    switch(value & 0xC0)
    {
    case 0x00: // PEN
        if ((value & 0x10) > 0)
            borderSelected = true;
        else
        {
            borderSelected = false;
            currentPen = value & 0x0F;
        }
        break;
    case 0x40: // INK
        if (borderSelected)
            BORDER = value & 0x1F;
        else
            INK[currentPen] = value & 0x1F;
        break;
    case 0x80: // RMR
        RMR = value & 0x3F;
        if ((value & 0x10) > 0)
        {
            R52 = 0;
            CPC::z80.InterruptRequest = true;
        }
        LoROMActive = (RMR & 0x04) != 0;
        HiROMActive = (RMR & 0x08) != 0;
        nextMode = RMR & 0x03;
        CPC::UpdateMemoryMap();
        break;
    case 0xC0: // MMR
        CPC::SelectRAM(value & 0x3F);
        break;
    }
}

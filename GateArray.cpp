#include "GateArray.h"
#include "CPC.h"
#include "CRTC.h"
#include "Z80.h"

const BYTE *GateArray::Color = Palette;
BYTE GateArray::INK[16];
BYTE GateArray::BORDER = 0;
BYTE GateArray::RMR;
BYTE GateArray::MMR;
BYTE GateArray::currentPen = 0;
bool GateArray::borderSelected = false;
word GateArray::videoAddress = 0;
word GateArray::currentWord = 0;
BYTE GateArray::pixelIndex = 0;
const BYTE *GateArray::blankColor = nullptr;
const BYTE *GateArray::currentPalette = GateArray::Palette;
bool GateArray::CCLK = false;
bool GateArray::VideoAccess = false;
bool GateArray::lastHSYNC = false;
bool GateArray::lastVSYNC = false;
bool GateArray::lastHDISP = false;
BYTE GateArray::R52 = 0;
BYTE GateArray::hsyncDelay = 0;
BYTE GateArray::vsyncDelay = 0;
BYTE GateArray::mode;
BYTE GateArray::pi;
BYTE GateArray::decodedPen[4][8][256];
bool GateArray::hsyncTrigger = false;
bool GateArray::vsyncTrigger = false;
bool GateArray::LoROMActive;
bool GateArray::HiROMActive;
bool GateArray::Monochrome;
BYTE GateArray::intTimeout;
BYTE GateArray::porch;
BYTE GateArray::ready;
BYTE GateArray::latchLo;
BYTE GateArray::latchHi;
bool GateArray::dispEnFF1;
bool GateArray::dispEnFF2;
BYTE GateArray::nextMode;

void GateArray::Reset()
{
    Color = AbsoluteBlack;
    for (int i = 0; i < 16; i++)
        INK[i] = 0;
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
    Z80::InterruptRequest = true;
}

void GateArray::ProcessSync()
{
    dispEnFF2 = dispEnFF1;
    dispEnFF1 = CRTC::BORDER;
    bool hsyncFallingEdge = lastHSYNC && !CRTC::HSYNC;
    bool hsyncRisingEdge = !lastHSYNC && CRTC::HSYNC;
    bool hdispRisingEdge = !lastHDISP && CRTC::HDISP;
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
            Z80::IRQ();
        }
        if (vsyncDelay > 0)
        {
            vsyncDelay--;
            if (vsyncDelay == 0)
            {
                vsyncTrigger = true;
                if (R52 >= 32) Z80::IRQ();
                R52 = 0;
            }
        }
    }
    if (!lastVSYNC && CRTC::VSYNC)
    {
        vsyncDelay = 2;
        porch = 26;
    }

    lastVSYNC = CRTC::VSYNC;
    lastHSYNC = CRTC::HSYNC;
    lastHDISP = CRTC::HDISP;

    if (CRTC::HSYNC || CRTC::VSYNC)
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
    videoAddress = (CRTC::MA & 0x03FF) << 1;
    videoAddress += (CRTC::RA & 0x07) << 11;
    videoAddress += (CRTC::MA & 0x3000) << 2;

    currentWord = latchHi * 256 + latchLo;
}

void GateArray::ReadByte(bool lo)
{
    int ramIndex = videoAddress >> 14;
    if (lo)
        latchLo = CPC::RAMs[ramIndex][videoAddress & 0x3FFF];
    else
        latchHi = CPC::RAMs[ramIndex][videoAddress & 0x3FFF];
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
            Z80::InterruptRequest = true;
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

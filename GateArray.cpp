#include "GateArray.h"
#include "CPC.h"
#include "CRTC.h"
#include "Z80.h"
#include "speedcontroller.h"

const BYTE *GateArray::Color = Palette;
BYTE GateArray::INK[16];
BYTE GateArray::BORDER = 0;
BYTE GateArray::RMR;
BYTE GateArray::MMR;
BYTE GateArray::currentPen = 0;
bool GateArray::borderSelected = false;
word GateArray::videoAddress = 0;
BYTE GateArray::currentByte = 0;
BYTE GateArray::pixelIndex = 0;
BYTE GateArray::currentInk = 0;
bool GateArray::CCLK = false;
bool GateArray::lastHSYNC = false;
bool GateArray::lastVSYNC = false;
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
    videoAddress = 0;
    currentByte = 0;
    pixelIndex = 0;
    lastHSYNC = false;
    lastVSYNC = false;
    hsyncDelay = 0;
    vsyncDelay = 0;
    intTimeout = 0;
    for (BYTE m = 0; m < 4; m++)
        for (BYTE i = 0; i < 8; i++)
            for (int b = 0; b < 256; b++)
                decodedPen[m][i][b] = GetPenForPixel(m, b, i);
}

void GateArray::Clock(int tick)
{
    // 1Mhz
    if ((tick % 16) == 0)
    {
        ProcessSync();
    }
    // 2 Mhz
    if ((tick % 8) == 0)
    {
        ReadByte();
        CCLK = !CCLK;
    }
    // 16 Mhz
    SetPixel();
}

void GateArray::AckInt()
{
    R52 &= 0x1F;
    Z80::InterruptRequest = true;
}

void GateArray::ProcessSync()
{

    bool hsyncFallingEdge = lastHSYNC && !CRTC::HSYNC;
    bool hsyncRisingEdge = !lastHSYNC && CRTC::HSYNC;
    bool vsyncRisingEdge = !lastVSYNC && CRTC::VSYNC;

    if (hsyncDelay > 0)
    {
        hsyncDelay--;
        if (hsyncDelay == 0)
            hsyncTrigger = true;
    }
    if (hsyncRisingEdge)
    {
        hsyncDelay = 2;
    }
    if (hsyncFallingEdge)
    {
        R52++;
        if (R52 == 52)
        {
            R52 = 0;
            Z80::InterruptRequest = false;
        }
        if (vsyncDelay > 0)
        {
            vsyncDelay--;
            if (vsyncDelay == 0)
            {
                vsyncTrigger = true;
                if (R52 >= 32)
                {
                    Z80::InterruptRequest = false;
                }
                R52 = 0;
            }
        }
    }

    if (vsyncRisingEdge)
    {
        vsyncDelay = 2;
    }

    lastVSYNC = CRTC::VSYNC;
    lastHSYNC = CRTC::HSYNC;
}

void GateArray::SetPixel()
{
    if (CRTC::HSYNC || CRTC::VSYNC)
    {
        Color = &AbsoluteBlack[0];
        return;
    }
    currentInk = CRTC::BORDER ? BORDER : INK[decodedPen[mode][pixelIndex][currentByte]];
    //if (SpeedController::overrun && BORDER)
//    if (CRTC::BORDER && !Z80::IFF1)
//        currentInk = 0;
    if (++pixelIndex == 8) pixelIndex = 0;
    if (Monochrome)
        Color = &GreenPalette[currentInk * 3];
    else
        Color = &Palette[currentInk * 3];
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

void GateArray::ReadByte()
{
    videoAddress = (CRTC::MA & 0x03FF) << 1;
    videoAddress += (CRTC::RA & 0x07) << 11;
    videoAddress += (CRTC::MA & 0x3000) << 2;
    videoAddress += CCLK;
    int ramIndex = videoAddress >> 14;
    currentByte = CPC::RAMs[ramIndex][videoAddress & 0x3FFF];
}

void GateArray::WR()
{
    switch(CPC::DataBUS & 0xC0)
    {
    case 0x00: // PEN
        if ((CPC::DataBUS & 0x10) > 0)
            borderSelected = true;
        else
        {
            borderSelected = false;
            currentPen = CPC::DataBUS & 0x0F;
        }
        break;
    case 0x40: // INK
        if (borderSelected)
            BORDER = CPC::DataBUS & 0x1F;
        else
            INK[currentPen] = CPC::DataBUS & 0x1F;
        break;
    case 0x80: // RMR
        RMR = CPC::DataBUS & 0x3F;
        if ((CPC::DataBUS & 0x10) > 0)
        {
            R52 = 0;
            Z80::InterruptRequest = true;
        }
        LoROMActive = (RMR & 0x04) != 0;
        HiROMActive = (RMR & 0x08) != 0;
        mode = RMR & 0x03;
        break;
    case 0xC0: // MMR
        CPC::SelectRAM(CPC::DataBUS & 0x3F);
        break;
    }
}

#include "Headers/GateArray.h"
#include "Headers/CPC.h"
#include "Headers/CRTC.h"
#include "Headers/Z80.h"
#include "Headers/PPI.h"
#include "Headers/PSG.h"
#include "Headers/ROMSelector.h"


const BYTE *GateArray::Color;
BYTE GateArray::INK[16];
BYTE GateArray::BORDER = 0;
BYTE GateArray::RMR;
BYTE GateArray::MMR;
BYTE GateArray::currentPen = 0;
bool GateArray::borderSelected = false;
word GateArray::videoAddress = 0;
BYTE GateArray::currentByte = 0;
BYTE GateArray::pixelIndex = 0;
BYTE GateArray::videoPen = 0;
BYTE GateArray::clockDividerCounter = 0;
bool GateArray::CCLK = false;
bool GateArray::lastHSYNC = false;
BYTE GateArray::R52 = 0;
BYTE GateArray::vsyncDelay = 0;
BYTE GateArray::hsyncDelay = 0;
BYTE GateArray::mode;
bool GateArray::GA_HSYNC;
bool GateArray::GA_VSYNC;


bool GateArray::ROMEN()
{
    if (Z80::MREQ || Z80::RD) return true;
    switch((CPC::AddressBUS & 0xC000) >> 14)
    {
    case 0:
        return (RMR & 0x04) != 0;
    case 3:
        return (RMR & 0x08) != 0;
    default:
        return true;
    }
}

bool GateArray::RAMRD()
{
    if (Z80::MREQ || Z80::RD) return true;
    switch((CPC::AddressBUS & 0xC000) >> 14)
    {
    case 0:
        return (RMR & 0x04) == 0;
    case 3:
        return (RMR & 0x08) == 0;
    default:
        return false;
    }
}

bool GateArray::MWE()
{
    if (Z80::MREQ || Z80::WR) return true;
    /*
    switch(CPC::bank())
    {
    case 3:       ///////////////////////////// For 6128
        return (RMR & 0x08) == 0;
    default:
        return false;
    }
    */
    return false;
}

void GateArray::Init()
{
    currentPen = 0;
    BORDER = 0;
    clockDividerCounter = 0;
    CCLK = false;
    currentByte = 0;
    pixelIndex = 0x80;
    GA_HSYNC = false;
    GA_HSYNC = true;
    RMR = 0;
    MMR = 0;
}

void GateArray::Clock()
{
    Z80::stopPoint = false;
    // 8 Mhz
    if ((clockDividerCounter & 0x01) == 0)
    {
        Z80::CLK = !Z80::CLK;
        Z80::ClockEdge();
    }

    // 4 Mhz
    if ((clockDividerCounter % 16) == 7)
    {
        IO_Clock();
        ROMSelector::IOClock();
        CRTC::IOClock();
        PPI::IOClock();
        CPC::ActiveROM()->Clock();
        CPC::InternalRAM->Clock();
    }

    if ((clockDividerCounter % 16) == 0)
    {
        PSG::Clock();
    }

    // 1Mhz
    if ((clockDividerCounter % 16) == 0)
    {
        CRTC::CRTClock();
        GenerateSyncAndInterrupt();
    }
    // 2 Mhz
    if ((clockDividerCounter % 8) == 0)
    {
        ReadByte();
        CCLK = !CCLK;
    }
    clockDividerCounter++;
    // 16 Mhz
    SetPixel();
}

void GateArray::GenerateSyncAndInterrupt()
{
    if (CRTC::HSYNC)
    {
        if (hsyncDelay)
            hsyncDelay--;
        else
        {
            if (GA_HSYNC == false)
            {
                mode = RMR & 0x03;
                R52++;
                if (R52 == 52)
                {
                    //////////////////////////////////////
                    /// CPU INT ACK Control
                    R52 = 0;
                    Z80::InterruptRequest = false;
                }
                if (CRTC::VSYNC)
                {
                    if (vsyncDelay)
                        vsyncDelay--;
                    else
                    {
                        if (GA_VSYNC == false)
                        {
                            if (R52 > 32)
                                Z80::InterruptRequest = false;
                            R52 = 0;
                        }
                        GA_VSYNC = true;
                    }
                }
                else
                {
                    vsyncDelay = 0;
                    GA_VSYNC = false;
                }
            }
            GA_HSYNC = true;
        }
    }
    else
    {
        hsyncDelay = 2;
        GA_HSYNC = false;
    }
}

void GateArray::SetPixel()
{
    if (CRTC::HSYNC)
    {
        Color = &Palette[60];
        return;
    }
    BYTE pi;
    switch(mode)
    {
    case 0:
        pi = pixelIndex >> 2;
        videoPen = ((currentByte & (0x80 >> pi)) > 0) +
                   ((currentByte & (0x20 >> pi)) > 0) * 4 +
                   ((currentByte & (0x08 >> pi)) > 0) * 2 +
                   ((currentByte & (0x02 >> pi)) > 0) * 8;
        break;
    case 1:
        pi = pixelIndex >> 1;
        videoPen = ((currentByte & (0x80 >> pi)) > 0) +
                   ((currentByte & (0x08 >> pi)) > 0) * 2;
        break;
    case 2:
        pi = pixelIndex;
        videoPen = (currentByte & (0x80 >> pi )) > 0;
        break;
    case 3:
        pi = pixelIndex >> 2;
        videoPen = ((pi & (0x08 >> pi)) >> (2 - pi))
                   + ((pi & (0x02 >> pi)) >> (1 - pi));
        break;
    }
    if (++pixelIndex == 8) pixelIndex = 0;
    Color = &Palette[(CRTC::BORDER ? BORDER : INK[videoPen]) * 3];
}

void GateArray::ReadByte()
{
    videoAddress = (CRTC::MA & 0x03FF) << 1;
    videoAddress += (CRTC::RA & 0x07) << 11;
    videoAddress += (CRTC::MA & 0x3000) << 2;
    videoAddress += CCLK;
    currentByte = CPC::InternalRAM->MEM[videoAddress];
}

void GateArray::IO_Clock()
{
    if (!Z80::IORQ && !Z80::WR && (CPC::AddressBUS & 0x8000) == 0)
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
            break;
        case 0xC0: // MMR
            MMR = CPC::DataBUS & 0x3F;
            break;
        }
    }
}

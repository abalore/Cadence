#include "Headers/GateArray.h"
#include "Headers/CPC.h"
#include "Headers/CRTC.h"
#include "Headers/Z80.h"
#include "Headers/PPI.h"
#include "Headers/PSG.h"
#include "Headers/ROMSelector.h"


BYTE GateArray::Color[3];
BYTE GateArray::INK[16];
BYTE GateArray::BORDER = 0;
BYTE GateArray::RMR = 0b00000000;
BYTE GateArray::MMR = 0;
BYTE GateArray::currentPen = 0;
bool GateArray::borderSelected = false;
word GateArray::videoAddress = 0;
BYTE GateArray::currentByte = 0;
BYTE GateArray::pixelIndex = 0;
BYTE GateArray::videoPen = 0;
BYTE GateArray::clockDividerCounter = 0;
bool GateArray::CCLK = false;
bool GateArray::lastHSYNC = false;
BYTE GateArray::hsyncCounter = 0;
BYTE GateArray::vsyncIntDelay = 0;
BYTE GateArray::mode;
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
    ROMSelector::Init();
    PPI::Init();
    PSG::Init();
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
        if (CRTC::HSyncFallingEdge)
        {
            mode = RMR & 0x03;
            hsyncCounter++;
            if (hsyncCounter == 52)// || (hsyncCounter >=32 && CRTC::VSYNCSTART))
            {
                //////////////////////////////////////
                /// CPU INT ACK Control
                hsyncCounter = 0;
                Z80::InterruptRequest = false;
            }
            if (CRTC::VSYNCSTART)
            {
                hsyncCounter = 0;
                if (hsyncCounter < 32)
                    vsyncIntDelay = 2;
            }
            if (vsyncIntDelay > 0)
            {
                Z80::InterruptRequest = false;
                if (vsyncIntDelay == 0)
                    Z80::InterruptRequest = false;
            }
        }
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

void GateArray::SetPixel()
{
    if (CRTC::HSYNC)
    {
        Color[0] = 0;
        Color[1] = 0;
        Color[2] = 0;
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

    int base = (CRTC::BORDER ? BORDER : INK[videoPen]) * 3;
    Color[0] = Palette[base];
    Color[1] = Palette[base + 1];
    Color[2] = Palette[base + 2];
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

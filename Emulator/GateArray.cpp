#include "Headers/GateArray.h"
#include "Headers/CPC.h"
#include "Headers/CRTC.h"
#include "Headers/Z80.h"
#include "Headers/PPI.h"

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
bool GateArray::CLK = false;
bool GateArray::CCLK = false;

bool GateArray::ROMEN()
{
    if (Z80::MREQ || Z80::RD) return true;
    switch(CPC::bank())
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
    switch(CPC::bank())
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
}

void GateArray::Clock()
{
    // 8 Mhz
    if ((clockDividerCounter % 2) == 0)
    {
        CLK = !CLK;
        Z80::ClockEdge(CLK);
    }
    // 4 Mhz
    if ((clockDividerCounter % 4) == 0)
    {
        IO_Clock();
        CPC::InternalRAM->Clock();
        CPC::ActiveROM()->Clock();
        CRTC::IOClock();
        PPI::IOClock();
    }
    // 1Mhz
    if ((clockDividerCounter % 16) == 0)
    {
        CRTC::CRTClock();
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
    if (CRTC::HSYNC || CRTC::VSYNC)
    {
        Color[0] = 0;
        Color[1] = 0;
        Color[2] = 0;
        return;
    }
    BYTE pi;
    switch(RMR & 0x03)
    {
    case 0:
        pi = pixelIndex >> 2;
        videoPen = ((currentByte & (0x80 >> pi)) >> (4 - pi))
                   + ((currentByte & (0x20 >> pi)) >> (3 - pi))
                   + ((currentByte & (0x08 >> pi)) >> (2 - pi))
                   + ((currentByte & (0x02 >> pi)) >> (1 - pi));
        break;
    case 1:
        pi = pixelIndex >> 1;
        videoPen = currentByte & (0x88 >> pi);
        videoPen >>= 3 - pi;
        videoPen <<= 1;
        videoPen = (videoPen & 0x02) + (videoPen >> 5);
        break;
    case 2:
        pi = pixelIndex;
        videoPen = (currentByte & (0x80 >> pi )) >> (7 - pi);
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
    videoAddress += CCLK ? 1 : 0;
    if (videoAddress == 0xC852)
    {
        CCLK = CCLK;
    }
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

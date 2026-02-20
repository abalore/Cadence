#include "Headers/GateArray.h"
#include "Headers/CPC.h"
#include "Headers/CRTC.h"
#include "Headers/Z80.h"
#include "Headers/PPI.h"
#include "Headers/PSG.h"
#include "Headers/ROMSelector.h"
#include "Headers/Tape.h"
#include "Headers/FDC.h"


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
BYTE GateArray::pi;
BYTE GateArray::decodedPen[4][8][256];
bool GateArray::hsyncTrigger = false;
bool GateArray::vsyncTrigger = false;
SyncState GateArray::hsyncState = SyncState::SSWaitingCRTC;
SyncState GateArray::vsyncState = SyncState::SSWaitingCRTC;


bool GateArray::ROMEN()
{
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
    for (BYTE m = 0; m < 4; m++)
        for (BYTE i = 0; i < 8; i++)
            for (int b = 0; b < 256; b++)
                decodedPen[m][i][b] = GetPenForPixel(m, b, i);
}

void GateArray::Clock()
{
    Z80::stopPoint = false;

    // 4 Mhz
    if ((clockDividerCounter % 4) == 2)
    {
        Z80::Clock();
        FDC::Clock();
    }

    // 1 Mhz

    if ((clockDividerCounter % 16) == 8)
    {
        if (!Z80::IORQ)
        {
            if (!Z80::RD)
            {
                if ((CPC::AddressBUS & 0x4000) == 0)
                    CRTC::Clock_IO_RD();
                else if ((CPC::AddressBUS & 0x0800) == 0)
                {
                    PSG::Clock_IO_RD();
                    PPI::Clock_IO_RD();
                }
                else if ((CPC::AddressBUS & 0x0480) == 0)
                    FDC::Clock_IO_RD();
            }
            else
                if (!Z80::WR)
                {
                    if ((CPC::AddressBUS & 0x8000) == 0)
                        Clock_IO_WR();
                    else if ((CPC::AddressBUS & 0x4000) == 0)
                        CRTC::Clock_IO_WR();
                    else if ((CPC::AddressBUS & 0x2000) == 0)
                        ROMSelector::Clock_IO_WR();
                    else if ((CPC::AddressBUS & 0x0800) == 0)
                    {
                        PPI::Clock_IO_WR();
                        PSG::Clock_IO_WR();
                    }
                    else if ((CPC::AddressBUS & 0x0480) == 0)
                        FDC::Clock_IO_WR();
                }
        }
        else
        {
            if (!Z80::MREQ)
            {
                if (!Z80::RD)
                {
                    if (!ROMEN())
                        CPC::ActiveROM()->Clock_RD();
                    else
                        CPC::ActiveRAM()->Clock_RD();
                }
                else
                    if (!Z80::WR)
                        CPC::ActiveRAM()->Clock_WR();
            }
        }
    }

    // 1Mhz
    if ((clockDividerCounter % 16) == 0)
    {
        Tape::Clock();
        PSG::Clock();
        CRTC::Clock();

        GenerateHSync();
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

void GateArray::GenerateVSync()
{
    switch(vsyncState)
    {
    case SyncState::SSWaitingCRTC:
        if (CRTC::VSYNC)
        {
            vsyncDelay = 2;
            vsyncState = SyncState::SSDelaying;
        }
        break;
    case SyncState::SSDelaying:
        vsyncDelay--;
        if (vsyncDelay == 0)
        {
            GA_VSYNC = true;
            vsyncTrigger = true;
            vsyncDelay = 2;
            vsyncState = SyncState::SSRunning;
            if (R52 > 32)
                Z80::InterruptRequest = false;
            R52 = 0;
        }
        break;
    case SyncState::SSRunning:
        vsyncDelay--;
        if (vsyncDelay == 0)
        {
            vsyncState = SyncState::SSFinished;
        }
        break;
    case SyncState::SSFinished:
        if (!CRTC::VSYNC)
        {
            GA_VSYNC = false;
            vsyncState = SyncState::SSWaitingCRTC;
        }
        break;
    }
}

void GateArray::GenerateHSync()
{
    switch(hsyncState)
    {
    case SyncState::SSWaitingCRTC:
        if (CRTC::HSYNC)
        {
            hsyncDelay = 2;
            hsyncState = SyncState::SSDelaying;
        }
        break;
    case SyncState::SSDelaying:
        hsyncDelay--;
        if (hsyncDelay == 0)
        {
            GA_HSYNC = true;
            hsyncDelay = 4;
            hsyncState = SyncState::SSRunning;
            hsyncTrigger = true;
        }
        break;
    case SyncState::SSRunning:
        hsyncDelay--;
        if (hsyncDelay == 0)
        {
            hsyncState = SyncState::SSFinished;
        }
        break;
    case SyncState::SSFinished:
        if (!CRTC::HSYNC)
        {
            GA_HSYNC = false;
            hsyncState = SyncState::SSWaitingCRTC;
            mode = RMR & 0x03;
            R52++;
            if (R52 == 52)
            {
                R52 = 0;
                Z80::InterruptRequest = false;
            }
        }
        break;
    }
}

void GateArray::SetPixel()
{
    if (CRTC::HSYNC)
    {
        Color = &AbsoluteBlack[0];
        return;
    }
    if (CRTC::BORDER)
    {
        //        if (Z80::InterruptEnable && !Z80::InterruptRequest)
        //            Color = &Palette[26 * 3];
        //        else
        Color = &Palette[BORDER * 3];
    }
    else
    {
        videoPen = decodedPen[mode][pixelIndex][currentByte];
        if (++pixelIndex == 8) pixelIndex = 0;
        Color = &Palette[INK[videoPen] * 3];
    }
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
    currentByte = CPC::BaseRAM.MEM[videoAddress];
}

void GateArray::Clock_IO_WR()
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
            R52 = 0;
        break;
    case 0xC0: // MMR
        MMR = CPC::DataBUS & 0x3F;
        break;
    }
}

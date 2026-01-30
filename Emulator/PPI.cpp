#include "Headers/PPI.h"
#include "Headers/Z80.h"
#include "Headers/CPC.h"
#include "Headers/GateArray.h"
#include "Headers/CRTC.h"
#include "Headers/PSG.h"
#include "Headers/Keyboard.h"

BYTE PPI::controlWord;

void PPI::Init()
{
    controlWord = 0;
}

void PPI::IOClock()
{
    if (!Z80::IORQ && (CPC::AddressBUS & 0x0800) == 0)
    {
        if (!Z80::WR)
        {
            switch((CPC::AddressBUS & 0x0300) >> 8)
            {
            case 0: // 8255 PPI Port A (PSG Data)   (R/W)
                if ((controlWord & 0x10) == 0)
                    CPC::psg->WriteData(CPC::DataBUS);
                break;
            case 1: // 8255 PPI Port B (Vsync,PrnBusy,Tape In,etc.) (R)
                break;
            case 2: // 8255 PPI Port C (KeybRow,Tape Out,PSG Control) (W)
                Keyboard::SetRow(CPC::DataBUS & 0x0F);
                CPC::psg->BDIR = (CPC::DataBUS & 0x80) > 0;
                CPC::psg->BC1 = (CPC::DataBUS & 0x40) > 0;
                break;
            case 3: // 8255 PPI Control-Register (W)
                if (CPC::DataBUS & 0x80)
                    controlWord = CPC::DataBUS;
                else
                {
                    BYTE bit = (CPC::DataBUS & 0x0E) >> 1;
                    BYTE value = (CPC::DataBUS & 0x01) << bit;
                    switch(value)
                    {
                    case 0x80:
                        CPC::psg -> BDIR = bit;
                        break;
                    case 0x40:
                        CPC::psg -> BC1 = bit;
                        break;
                    case 0x20:
                        break;
                    case 0x10:
                        break;
                    default:
                        Keyboard::Row &= value ^0xFF;
                        Keyboard::Row |= value;
                        break;
                    }
                }
                break;
            }
        }
        if (!Z80::RD)
        {
            switch((CPC::AddressBUS & 0x0300) >> 8)
            {
            case 0: // 8255 PPI Port A (PSG Data)   (R/W)
                CPC::DataBUS = (controlWord & 0x10) ? CPC::psg -> ReadData() : 0xFF;
                break;
            case 1: // 8255 PPI Port B (Vsync,PrnBusy,Tape In,etc.) (R)
                CPC::DataBUS = 0x3E + CRTC::VSYNC;
                break;
            case 2: // 8255 PPI Port C (KeybRow,Tape Out,PSG Control) (W)
                CPC::DataBUS = 0xFF;
                break;
            case 3: // 8255 PPI Control-Register (W)
                CPC::DataBUS = 0xFF;
                break;
            }
        }
    }
}

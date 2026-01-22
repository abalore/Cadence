#include "Headers/PPI.h"
#include "Headers/Z80.h"
#include "Headers/CPC.h"
#include "Headers/CRTC.h"

void PPI::Init()
{

}

void PPI::IOClock()
{
    if (!Z80::IORQ && (CPC::AddressBUS & 0x0800) == 0)
    {
        if (!(Z80::WR && Z80::RD))
        {
            switch((CPC::AddressBUS & 0x0300) >> 8)
            {
            case 0: // 8255 PPI Port A (PSG Data)   (R/W)
                break;
            case 1: // 8255 PPI Port B (Vsync,PrnBusy,Tape,etc.) (R)
                if (!Z80::RD)
                    CPC::DataBUS = 0x3E + CRTC::VSYNC;
                break;
            case 2: // 8255 PPI Port C (KeybRow,Tape,PSG Control) (W)
                break;
            case 3: // 8255 PPI Control-Register (W)
                break;
            }
        }
    }
}

#include "Headers/ROMSelector.h"
#include "Headers/CPC.h"
#include "Headers/Z80.h"

BYTE ROMSelector::SelectedROM;

void ROMSelector::Init()
{
    SelectedROM = 0;
}

void ROMSelector::IOClock()
{
    if (!Z80::IORQ && (CPC::AddressBUS & 0x2000) == 0)
    {
        if (!Z80::WR)
            SelectedROM = CPC::DataBUS;
    }
}

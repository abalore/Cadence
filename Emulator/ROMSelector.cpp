#include "Headers/ROMSelector.h"
#include "Headers/CPC.h"
#include "Headers/Z80.h"

BYTE ROMSelector::SelectedROM;

void ROMSelector::Init()
{
    SelectedROM = 0;
}

void ROMSelector::Clock_IO_WR()
{
    SelectedROM = CPC::DataBUS & 0x1F;
}

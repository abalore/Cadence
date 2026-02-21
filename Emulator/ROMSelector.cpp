#include "Headers/ROMSelector.h"
#include "Headers/CPC.h"
#include "Headers/Z80.h"

BYTE ROMSelector::SelectedROM;

void ROMSelector::Init()
{
    SelectedROM = 0;
}

void ROMSelector::WR()
{
    SelectedROM = Z80::DR & 0x1F;
    CPC::HiROM = CPC::HiROMs[SelectedROM];
}

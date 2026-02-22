#include "Headers/CPC.h"
#include "Headers/GateArray.h"
#include "Headers/Keyboard.h"
#include "Headers/ROMSelector.h"
#include "Headers/CRTC.h"
#include "Headers/Z80.h"
#include "Headers/PPI.h"
#include "Headers/PSG.h"
#include "Headers/FDC.h"
#include <stdlib.h>

BYTE *CPC::RAM[4];
BYTE *CPC::LoROM;
BYTE *CPC::HiROM;
BYTE *CPC::HiROMs[32];
BYTE *CPC::RAMs[8];
BYTE CPC::bank;
word CPC::addr;

void CPC::Init()
{
    for (int i = 0; i < 32; i++) HiROMs[i] = nullptr;
    for (int i = 0; i < 8; i++) RAMs[i] = nullptr;
    LoROM = nullptr;
}

void CPC::Finalize()
{
    for (int i = 0; i < 32; i++) if (HiROMs[i] != nullptr)
    {
        free(HiROMs[i]);
        HiROMs[i] = nullptr;
    }
    for (int i = 0; i < 8; i++) if (RAMs[i] != nullptr)
    {
        free(RAMs[i]);
        RAMs[i] = nullptr;
    }
    if (LoROM != nullptr) free(LoROM);
}

void CPC::Clock()
{
    GateArray::Clock();
}

void CPC::Reset()
{
    for (int i = 0; i < 4; i++) RAM[i] = RAMs[i];
    Z80::Init();
    PPI::Init();
    CRTC::Init();
    GateArray::Init();
    Keyboard::Init();
    ROMSelector::Init();
    PPI::Init();
    PSG::Init();
    FDC::Reset();
}

BYTE CPC::GetRAMByteAt(word address)
{
    int bank = address >> 14;
    int addr = address & 0x3FFF;
    return RAM[bank][addr];
}

BYTE CPC::GetByteAt(word address)
{
    bank = address >> 14;
    addr = address & 0x3FFF;
    switch(bank)
    {
    case 0:
        if (!GateArray::LoROMActive)
            return LoROM[addr];
        else
            return RAM[bank][addr];
        break;
    case 1:
    case 2:
        return RAM[bank][addr];
        break;
    case 3:
        if (!GateArray::HiROMActive)
            return HiROM[addr];
        else
            return RAM[bank][addr];
    }
    return 0;
}

void CPC::SetByteAt(word address, BYTE b)
{
    int bank = address >> 14;
    int addr = address & 0x3FFF;
    RAM[bank][addr] = b;
}

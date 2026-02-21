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

BYTE *CPC::BaseRAM;
BYTE *CPC::LoROM;
BYTE *CPC::HiROM;
BYTE *CPC::HiROMs[32];

BYTE *CPC::ActiveRAM()
{
    return BaseRAM;
}

BYTE *CPC::ActiveROM()
{
    if (Z80::AR >= 0xC000)
    {
        return HiROM;
    }
    else
        return LoROM;
}

void CPC::Init()
{
    BaseRAM = (BYTE *) malloc(0x10000);
    LoROM = (BYTE *) malloc(0x10000);
    for (int i = 0; i < 32; i++)
        HiROMs[i] = (BYTE *) malloc(0x10000);
    HiROM = HiROMs[0];
    Reset();
}

void CPC::Clock()
{
    GateArray::Clock();
}

void CPC::Reset()
{
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


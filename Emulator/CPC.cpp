#include "Headers/CPC.h"
#include "Headers/ROM.h"
#include "Headers/CRTScreen.h"
#include "Headers/GateArray.h"
#include "Headers/Keyboard.h"
#include "Headers/ROMSelector.h"
#include "Headers/CRTC.h"
#include "Headers/Z80.h"
#include "Headers/PPI.h"
#include "Headers/PSG.h"
#include "Headers/FDC.h"

word CPC::AddressBUS;
BYTE CPC::DataBUS;
RAM CPC::BaseRAM;
ROM CPC::LoROM(0x0000);
ROM CPC::HiROM[32] = {
    ROM(0xC000), ROM(0xC000), ROM(0xC000), ROM(0xC000),
    ROM(0xC000), ROM(0xC000), ROM(0xC000), ROM(0xC000),
    ROM(0xC000), ROM(0xC000), ROM(0xC000), ROM(0xC000),
    ROM(0xC000), ROM(0xC000), ROM(0xC000), ROM(0xC000),
    ROM(0xC000), ROM(0xC000), ROM(0xC000), ROM(0xC000),
    ROM(0xC000), ROM(0xC000), ROM(0xC000), ROM(0xC000),
    ROM(0xC000), ROM(0xC000), ROM(0xC000), ROM(0xC000),
    ROM(0xC000), ROM(0xC000), ROM(0xC000), ROM(0xC000)
};

RAM *CPC::ActiveRAM()
{
    return &BaseRAM;
}

ROM *CPC::ActiveROM()
{
    if ((AddressBUS & 0xC000) > 0)
    {
        return &HiROM[ROMSelector::SelectedROM];
    }
    else
        return &LoROM;
}

void CPC::Init()
{
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


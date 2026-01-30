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

word CPC::AddressBUS;
BYTE CPC::DataBUS;
RAM *CPC::InternalRAM;
ROM *CPC::LoROM;
ROM *CPC::HiROM;
ROM *CPC::ExpansionROM;
PSG *CPC::psg;

ROM *CPC::ActiveROM()
{
    if ((AddressBUS & 0xC000) > 0)
    {
        switch(ROMSelector::SelectedROM)
        {
        case 1:
            return ExpansionROM;
        default:
            return HiROM;
        }
    }
    else
        return LoROM;
}

void CPC::Init()
{
    InternalRAM = new RAM();
    LoROM = new ROM(0xFF); // -1 = lower
    HiROM = new ROM(0);
    ExpansionROM = new ROM(1);
    psg = new PSG();
    Reset();
}

void CPC::Clock()
{
    GateArray::Clock();
    CRTScreen::Clock();
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
    psg->Init();
}

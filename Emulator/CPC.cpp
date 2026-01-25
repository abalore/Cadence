#include "Headers/CPC.h"
#include "Headers/ROM.h"
#include "Headers/CRTScreen.h"
#include "Headers/GateArray.h"
#include "Headers/Keyboard.h"
#include "Headers/ROMSelector.h"
#include <QElapsedTimer>

word CPC::AddressBUS;
BYTE CPC::DataBUS;
RAM *CPC::InternalRAM;
ROM *CPC::LoROM;
ROM *CPC::HiROM;
ROM *CPC::ExpansionROM;

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

BYTE CPC::bank()
{
    return (BYTE)((AddressBUS & 0xC000) >> 14);
}

void CPC::Init()
{
    InternalRAM = new RAM();
    LoROM = new ROM(0xFF); // -1 = lower
    HiROM = new ROM(0);
    ExpansionROM = new ROM(1);
    GateArray::Init();
    Keyboard::Init();
}

void CPC::Clock()
{
    GateArray::Clock();
    CRTScreen::Clock();
}

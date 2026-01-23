#include "Headers/CPC.h"
#include "Headers/ROM.h"
#include "Headers/CRTScreen.h"
#include "Headers/GateArray.h"
#include "Headers/Keyboard.h"
#include <QElapsedTimer>

word CPC::AddressBUS;
BYTE CPC::DataBUS;
RAM *CPC::InternalRAM;
ROM *CPC::LoROM;
ROM *CPC::HiROM;

ROM *CPC::ActiveROM()
{
    if ((AddressBUS & 0xC000) > 0)
        return HiROM;
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
    LoROM = new ROM(0x0000);
    HiROM = new ROM(0xC000);
    GateArray::Init();
    Keyboard::Init();
}

void CPC::Clock()
{
    GateArray::Clock();
    CRTScreen::Clock();
}

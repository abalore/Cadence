#include "Headers/ROM.h"
#include "Headers/CPC.h"
#include "Headers/GateArray.h"

ROM::ROM(BYTE number)
{
    MEM = new BYTE[16384];
    if (number == 0xFF)
        Location = 0x0000;
    else
        Location = 0xC000;
    Number = number;
}

void ROM::Clock()
{
    if (!GateArray::ROMEN())
        CPC::DataBUS = MEM[CPC::AddressBUS - Location];
}

#include "Headers/ROM.h"
#include "Headers/CPC.h"
#include "Headers/Z80.h"
#include "Headers/GateArray.h"
#include <cstring>

ROM::ROM(word location)
{
    MEM = new BYTE[16384];
    Location = location;
}

void ROM::Clock()
{
    if (!GateArray::ROMEN())
        CPC::DataBUS = MEM[CPC::AddressBUS - Location];
}

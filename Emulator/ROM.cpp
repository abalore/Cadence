#include "Headers/ROM.h"
#include "Headers/CPC.h"

ROM::ROM(word location)
{
    MEM = new BYTE[16384];
    Location = location;
}

void ROM::Clock_RD()
{
    CPC::DataBUS = MEM[CPC::AddressBUS - Location];
}

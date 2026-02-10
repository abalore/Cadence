#include "Headers/RAM.h"
#include "Headers/CPC.h"
#include "Headers/Z80.h"
#include "Headers/GateArray.h"
#include <cstring>

//BYTE RAM::MEM[65536];

RAM::RAM()
{
    //MEM = new BYTE[65536];
}

void RAM::Clock_RD()
{
    CPC::DataBUS = MEM[CPC::AddressBUS];
}

void RAM::Clock_WR()
{
    MEM[CPC::AddressBUS] = CPC::DataBUS;
}

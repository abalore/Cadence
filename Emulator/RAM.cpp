#include "Headers/RAM.h"
#include "Headers/CPC.h"
#include "Headers/Z80.h"
#include "Headers/GateArray.h"
#include <cstring>

RAM::RAM()
{
    MEM = new BYTE[65536];
}

void RAM::Clock()
{
    if (!GateArray::RAMRD())
        CPC::DataBUS = MEM[CPC::AddressBUS];
    else
        if (!GateArray::MWE())
            MEM[CPC::AddressBUS] = CPC::DataBUS;
}

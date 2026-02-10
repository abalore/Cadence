#include "Headers/FDC.h"
#include "Headers/CPC.h"

void FDC::Clock_IO_RD()
{
    if ((CPC::AddressBUS & 0x0100) != 0)
    {
        if ((CPC::AddressBUS & 0x0001) == 0)
        {
            // Read Status
        }
        else
        {
            // Read data
        }
    }
}

void FDC::Clock_IO_WR()
{
    if ((CPC::AddressBUS & 0x0100) == 0)
    {
        // Set motor
    }
    else if ((CPC::AddressBUS & 0x0001) != 0)
    {
        // Write data
    }
}

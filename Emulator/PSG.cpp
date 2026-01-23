#include "Headers/PSG.h"
#include "Headers/Keyboard.h"

bool PSG::BC1;
bool PSG::BDIR;
BYTE PSG::inputRegister;
BYTE PSG::outputRegister;
BYTE PSG::selectedRegister;
BYTE PSG::registers[16];

void PSG::Init()
{
    BDIR = false;
    BC1 = true;
}

void PSG::Clock()
{
    if (BDIR)
    {
        if (BC1)
            selectedRegister = inputRegister;
        else
        {
                registers[selectedRegister] = inputRegister;
        }
    }
    else
    {
        if (BC1)
        {
            if (selectedRegister < 0x0E)
                outputRegister = registers[selectedRegister];
            else if (selectedRegister == 0x0E)
                outputRegister = Keyboard::Read();
        }
        else
            outputRegister = 0xFF;
    }
}

BYTE PSG::ReadData()
{
    return outputRegister;
}

void PSG::WriteData(BYTE data)
{
    inputRegister = data;
}

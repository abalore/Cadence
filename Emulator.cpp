#include "Emulator.h"
#include "CPC.h"
#include "Disassembler.h"

bool Emulator::Breakpoint[65536];

void Emulator::Init()
{
    CPC::Init();
    Disassembler::Init();
}

void Emulator::Finalize()
{
    CPC::Finalize();
}

void Emulator::Clock()
{
    CPC::Clock();
}

void Emulator::Reset()
{
    CPC::Reset();
    for (int i = 0; i < 65536; i++)
        Breakpoint[i] = false;
}

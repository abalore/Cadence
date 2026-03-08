#include "Headers/Emulator.h"
#include "Headers/CPC.h"
#include "Headers/Tape.h"
#include "Headers/Disassembler.h"
#include "Headers/CRTScreen.h"

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
}

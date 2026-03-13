#include "Emulator.h"
#include "CPC.h"
#include "Tape.h"
#include "Disassembler.h"
#include "CRTScreen.h"

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

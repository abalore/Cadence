#include "Headers/Emulator.h"
#include "Headers/CPC.h"
#include "Headers/Tape.h"
#include "Headers/Disassembler.h"
#include "Headers/CRTScreen.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

void Emulator::ReadROM(char *filename, int number)
{
    BYTE *dest;
    if (number == -1)
        dest = CPC::LoROM.MEM;
    else
        dest = CPC::HiROM[number].MEM;
    FILE *file = fopen(filename, "r");
    if (!fread(dest, 1, 16384, file)) {}
    fclose(file);
}

void Emulator::Init()
{
    Disassembler::Init();
    CPC::Init();
    ReadROM((char *)"ROM/ROM_BIOS_6128.bin", -1);
    ReadROM((char *)"ROM/ROM_BASIC_6128.bin", 0);
    ReadROM((char *)"ROM/ROM_AMSDOS_6128.bin", 7);
}

void Emulator::Clock()
{
    CPC::Clock();
    CRTScreen::Clock();
}

void Emulator::Reset()
{
    CPC::Reset();
}

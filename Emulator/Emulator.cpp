#include "Headers/Emulator.h"
#include "Headers/CPC.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

void ReadROM(char *filename, BYTE *dest)
{
    FILE *file = fopen(filename, "r");
    fread(dest, 1, 16384, file);
    fclose(file);
}

void Emulator::Init()
{
    CPC::Init();
    ReadROM((char *)"ROM/ROM_BIOS_464.bin", CPC::LoROM->MEM);
    ReadROM((char *)"ROM/ROM_BASIC_464.bin", CPC::HiROM->MEM);
}

void Emulator::Clock()
{
    CPC::Clock();
}


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
    ReadROM((char *)"ROM/ROM_BIOS_6128.bin", CPC::LoROM->MEM);
    ReadROM((char *)"ROM/ROM_BASIC_6128.bin", CPC::HiROM->MEM);
    ReadROM((char *)"ROM/ROM_OH_MUMMY.bin", CPC::ExpansionROM->MEM);
    //ReadROM((char *)"ROM/ROM_BOULDER_DASH.bin", CPC::ExpansionROM->MEM);
    //ReadROM((char *)"ROM/ROM_BRUCE_LEE.bin", CPC::ExpansionROM->MEM);
    //ReadROM((char *)"ROM/ROM_DONKEY_KONG.bin", CPC::ExpansionROM->MEM);
}

void Emulator::Clock()
{
    CPC::Clock();
}


#include "Headers/Emulator.h"
#include "Headers/CPC.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

void Emulator::ReadROM(char *filename, BYTE *dest)
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

    // INT ok
    ReadROM((char *)"ROM/ROM_OH_MUMMY.bin", CPC::ExpansionROM->MEM); // Collection broken
    //ReadROM((char *)"ROM/ROM_BOULDER_DASH.bin", CPC::ExpansionROM->MEM); // Gameplay broken at start
    //ReadROM((char *)"ROM/ROM_BRUCE_LEE.bin", CPC::ExpansionROM->MEM); // WORKING
    //ReadROM((char *)"ROM/ROM_DONKEY_KONG.bin", CPC::ExpansionROM->MEM); // WORKING
    //ReadROM((char *)"ROM/Ahhh.rom", CPC::ExpansionROM->MEM); // WORKING
    //ReadROM((char *)"ROM/Airwolf.rom", CPC::ExpansionROM->MEM); // WORKING
    //ReadROM((char *)"ROM/Anarchy.rom", CPC::ExpansionROM->MEM); // Crash after the menu
    //ReadROM((char *)"ROM/Arkanoid.rom", CPC::ExpansionROM->MEM); // WORKING
    //ReadROM((char *)"ROM/Manic_Miner.rom", CPC::ExpansionROM->MEM); // WORKING
    //ReadROM((char *)"ROM/Moon_Buggy.rom", CPC::ExpansionROM->MEM); // WORKING
    //ReadROM((char *)"ROM/Tempest.rom", CPC::ExpansionROM->MEM); // WORKING
    //ReadROM((char *)"ROM/Thrust.rom", CPC::ExpansionROM->MEM); // Controls fail
    //ReadROM((char *)"ROM/AmstradDiagUpper.rom", CPC::ExpansionROM->MEM); // WORKING
}

void Emulator::Clock()
{
    CPC::Clock();
}

void Emulator::Reset()
{
    CPC::Reset();
}

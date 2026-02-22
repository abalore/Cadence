#include "Headers/Emulator.h"
#include "Headers/CPC.h"
#include "Headers/Tape.h"
#include "Headers/Disassembler.h"
#include "Headers/CRTScreen.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

CPCType Emulator::cpcType = CPCType::CPC6128;

void Emulator::ReadROM(char *filename, int number)
{
    BYTE *dest;
    if (number == -1)
    {
        if (CPC::LoROM != nullptr) free(CPC::LoROM);
        CPC::LoROM = (BYTE *) malloc(0x4000);
        dest = CPC::LoROM;
    }
    else
    {
        if (CPC::HiROMs[number] != nullptr) free(CPC::HiROMs[number]);
        CPC::HiROMs[number] = (BYTE *) malloc(0x4000);
        dest = CPC::HiROMs[number];
    }
    FILE *file = fopen(filename, "r");
    if (!fread(dest, 1, 16384, file)) {}
    fclose(file);
}

void Emulator::Init()
{
    CPC::Init();
    switch(cpcType)
    {
    case CPCType::CPC464:
        ReadROM((char *)"ROM/ROM_BIOS_464.bin", -1);
        ReadROM((char *)"ROM/ROM_BASIC_464.bin", 0);
        for (int i = 0; i < 4; i++)
        {
            CPC::RAMs[i] = (BYTE *) malloc(0x4000);
        }
        break;
    case CPCType::CPC664:
        ReadROM((char *)"ROM/ROM_BIOS_664.bin", -1);
        ReadROM((char *)"ROM/ROM_BASIC_664.bin", 0);
        ReadROM((char *)"ROM/ROM_AMSDOS_6128.bin", 7);
        for (int i = 0; i < 4; i++)
            CPC::RAMs[i] = (BYTE *) malloc(0x4000);
        break;
    case CPCType::CPC6128:
        ReadROM((char *)"ROM/ROM_BIOS_6128.bin", -1);
        ReadROM((char *)"ROM/ROM_BASIC_6128.bin", 0);
        ReadROM((char *)"ROM/ROM_AMSDOS_6128.bin", 7);
        for (int i = 0; i < 8; i++)
            CPC::RAMs[i] = (BYTE *) malloc(0x4000);
        break;
    }

    Disassembler::Init();
}

void Emulator::Finalize()
{
    CPC::Finalize();
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

#include "CPC.h"
#include "GateArray.h"
#include "Keyboard.h"
#include "CRTC.h"
#include "Z80.h"
#include "PPI.h"
#include "PSG.h"
#include "FDC.h"
#include "Tape.h"
#include "CRTScreen.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

BYTE *CPC::RAM[4];
BYTE *CPC::LoROM;
BYTE *CPC::HiROM;
BYTE *CPC::HiROMs[ROM_SLOTS];
BYTE *CPC::RAMs[8];
BYTE CPC::bank;
word CPC::addr;
BYTE *CPC::Cartridge;
bool CPC::cartridgeEnabled;
CPCType CPC::cpcType = CPCType::CPC6128;
BYTE CPC::tick = 0;

void CPC::ReadROM(char *filename, int number)
{
    BYTE *dest;
    if (number == -1)
    {
        if (LoROM != nullptr) free(LoROM);
        LoROM = (BYTE *) malloc(0x4000);
        dest = LoROM;
    }
    else
    {
        if (HiROMs[number] != nullptr) free(HiROMs[number]);
        HiROMs[number] = (BYTE *) malloc(0x4000);
        dest = HiROMs[number];
    }
    FILE *file = fopen(filename, "r");
    if (!fread(dest, 1, 16384, file)) {}
    fclose(file);
}

void CPC::ReadCartridge(char *filename)
{
    Cartridge = (BYTE *)malloc(524288);
    FILE *file = fopen(filename, "r");
    if (!fread(Cartridge, 1, 524288, file)) {}
    fclose(file);
}

void CPC::Init()
{
    tick = 0;

    for (int i = 0; i < ROM_SLOTS; i++) HiROMs[i] = nullptr;
    Cartridge = nullptr;
    for (int i = 0; i < 8; i++) RAMs[i] = nullptr;
    LoROM = nullptr;

    switch(cpcType)
    {
    case CPCType::CPC464:
        ReadROM((char *)"ROM/ROM_BIOS_464.bin", -1);
        ReadROM((char *)"ROM/ROM_BASIC_464.bin", 0);
        for (int i = 0; i < 4; i++)
        {
            RAMs[i] = (BYTE *) malloc(0x4000);
        }
        break;
    case CPCType::CPC664:
        ReadROM((char *)"ROM/ROM_BIOS_664.bin", -1);
        ReadROM((char *)"ROM/ROM_BASIC_664.bin", 0);
        ReadROM((char *)"ROM/ROM_AMSDOS_6128.bin", 7);
        for (int i = 0; i < 4; i++)
            RAMs[i] = (BYTE *) malloc(0x4000);
        break;
    case CPCType::CPC6128:
        ReadROM((char *)"ROM/ROM_BIOS_6128.bin", -1);
        ReadROM((char *)"ROM/ROM_BASIC_6128.bin", 0);
        ReadROM((char *)"ROM/ROM_AMSDOS_6128.bin", 7);
        for (int i = 0; i < 8; i++)
            RAMs[i] = (BYTE *) malloc(0x4000);
        break;
    }
}

void CPC::Finalize()
{
    for (int i = 0; i < ROM_SLOTS; i++) if (HiROMs[i] != nullptr)
        {
            free(HiROMs[i]);
            HiROMs[i] = nullptr;
        }
    if (Cartridge != nullptr)
        free(Cartridge);
    for (int i = 0; i < 8; i++) if (RAMs[i] != nullptr)
        {
            free(RAMs[i]);
            RAMs[i] = nullptr;
        }
    if (LoROM != nullptr) free(LoROM);
}

void CPC::Clock()
{
    Z80::stopPoint = false;
    if ((tick % 16) == 0)
    {
        Z80::Clock();
        FDC::Clock();
        Tape::Clock();
        PSG::Clock();
        CRTC::Clock();
        //CRTScreen::OneMhzClock();
    }
    GateArray::Clock(tick);
    if ((tick % 16) == 0)
    {
        Z80::Clock2();
    }
    CRTScreen::Clock();
    tick++;
}

void CPC::Reset()
{
    tick = 0;
    SelectRAM(0);
    SelectROM(0);
    Z80::Reset();
    PPI::Reset();
    CRTC::Reset();
    GateArray::Reset();
    Keyboard::Reset();
    PPI::Reset();
    PSG::Reset();
    FDC::Reset();
}

BYTE CPC::GetRAMByteAt(word address)
{
    int bank = address >> 14;
    int addr = address & 0x3FFF;
    return RAM[bank][addr];
}

BYTE CPC::GetByteAt(word address)
{
    bank = address >> 14;
    addr = address & 0x3FFF;
    switch(bank)
    {
    case 0:
        if (!GateArray::LoROMActive)
        {
            if (cartridgeEnabled)
            {
                if (Cartridge == nullptr)
                    return 0;
                return Cartridge[addr];
            }
            return LoROM[addr];
        }
        else
            return RAM[bank][addr];
        break;
    case 1:
    case 2:
        return RAM[bank][addr];
        break;
    case 3:
        if (!GateArray::HiROMActive)
        {
            if (HiROM == nullptr)
                return 0;
            return HiROM[addr];
        }
        else
            return RAM[bank][addr];
    }
    return 0;
}

void CPC::SetByteAt(word address, BYTE b)
{
    int bank = address >> 14;
    int addr = address & 0x3FFF;
    RAM[bank][addr] = b;
}

void CPC::SelectROM(BYTE number)
{
    if (cartridgeEnabled)
    {
        HiROM = nullptr;
        if (Cartridge == nullptr)
            return;
        if (number >= 0x80 && number <= 0x9F)
            HiROM = Cartridge + (number - 0x80) * 0x4000;
        else if (number == 0)
            HiROM = Cartridge + 0x4000;
        else if (number == 7)
            HiROM  = Cartridge + 0x8000;
        else if (number < 16)
            HiROM = Cartridge + number * 0x4000;
    }
    else
    {
        if (number < ROM_SLOTS && HiROMs[number] != nullptr)
            HiROM = HiROMs[number];
        else
            HiROM = HiROMs[0];
    }
}

void CPC::SelectRAM(BYTE mmr)
{
    if (cpcType == CPCType::CPC6128)
    {
        switch(mmr & 0x07)
        {
        case 0:
            RAM[0] = RAMs[0]; RAM[1] = RAMs[1]; RAM[2] = RAMs[2]; RAM[3] = RAMs[3];
            break;
        case 1:
            RAM[0] = RAMs[0]; RAM[1] = RAMs[1]; RAM[2] = RAMs[2]; RAM[3] = RAMs[7];
            break;
        case 2:
            RAM[0] = RAMs[4]; RAM[1] = RAMs[5]; RAM[2] = RAMs[6]; RAM[3] = RAMs[7];
            break;
        case 3:
            RAM[0] = RAMs[0]; RAM[1] = RAMs[3]; RAM[2] = RAMs[2]; RAM[3] = RAMs[7];
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            RAM[0] = RAMs[0]; RAM[1] = RAMs[mmr & 0x07]; RAM[2] = RAMs[2]; RAM[3] = RAMs[3];
            break;
        }
    }
}

#include "CPC.h"
#include "GateArray.h"
#include "Keyboard.h"
#include "Z80.h"
#include "PPI.h"
#include "PSG.h"
#include "FDC.h"
#include "Tape.h"
#include "CRTScreen.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <cstring>

bool CPC::Breakpoint[65536];
CRTC CPC::crtc;
Keyboard CPC::keyboard;
Tape CPC::tape;
FDC CPC::fdc;
PSG CPC::psg;
PPI CPC::ppi;
CRTScreen CPC::screen;
GateArray CPC::gateArray;
Z80 CPC::z80;
BYTE *CPC::RAM[4];
BYTE *CPC::memPage[4];
BYTE CPC::zeroPage[0x4000] = {};
BYTE *CPC::LoROM;
BYTE *CPC::HiROM;
BYTE *CPC::HiROMs[ROM_SLOTS];
BYTE *CPC::RAMs[8];
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
    char fullpath[1024];
    const char *home = getenv("HOME");
    if (filename[0] != '/' && home)
        snprintf(fullpath, sizeof(fullpath), "%s/.cadence/%s", home, filename);
    else
        snprintf(fullpath, sizeof(fullpath), "%s", filename);
    FILE *file = fopen(fullpath, "r");
    if (file)
    {
        size_t n = fread(dest, 1, 16384, file);
        (void)n;
        fclose(file);
    }
}

void CPC::ClearROM(int number)
{
    if (number < 0 || number >= ROM_SLOTS) return;
    if (HiROMs[number] != nullptr)
    {
        free(HiROMs[number]);
        HiROMs[number] = nullptr;
    }
}

void CPC::ReadCartridge(char *filename)
{
    if (Cartridge != nullptr) free(Cartridge);
    Cartridge = (BYTE *)malloc(524288);
    FILE *file = fopen(filename, "r");
    if (file)
    {
        size_t n = fread(Cartridge, 1, 524288, file);
        (void)n;
        fclose(file);
    }
}

void CPC::InsertBlankCartridge()
{
    if (Cartridge != nullptr) free(Cartridge);
    Cartridge = (BYTE *)malloc(524288);
    memset(Cartridge, 0, 524288);
}

void CPC::Init()
{
    tick = 0;

    auto allocRAM = [](int count) {
        for (int i = 0; i < 8; i++) if (RAMs[i] != nullptr)
        {
            free(RAMs[i]);
            RAMs[i] = nullptr;
        }
        for (int i = 0; i < count; i++)
            RAMs[i] = (BYTE *) malloc(0x4000);
    };

    switch(cpcType)
    {
    case CPCType::CPC464:
        ReadROM((char *)"ROM/ROM_BIOS_464.bin", -1);
        ReadROM((char *)"ROM/ROM_BASIC_464.bin", 0);
        allocRAM(4);
        break;
    case CPCType::CPC664:
        ReadROM((char *)"ROM/ROM_BIOS_664.bin", -1);
        ReadROM((char *)"ROM/ROM_BASIC_664.bin", 0);
        ReadROM((char *)"ROM/ROM_AMSDOS_6128.bin", 7);
        allocRAM(4);
        break;
    case CPCType::CPC6128:
        ReadROM((char *)"ROM/ROM_BIOS_6128.bin", -1);
        ReadROM((char *)"ROM/ROM_BASIC_6128.bin", 0);
        ReadROM((char *)"ROM/ROM_AMSDOS_6128.bin", 7);
        //ReadROM((char *)"ROM/PARADOS.ROM", 7);
        allocRAM(8);
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
    {
        free(Cartridge);
        Cartridge = nullptr;
    }
    for (int i = 0; i < 8; i++) if (RAMs[i] != nullptr)
        {
            free(RAMs[i]);
            RAMs[i] = nullptr;
        }
    if (LoROM != nullptr)
    {
        free(LoROM);
        LoROM = nullptr;
    }
    HiROM = nullptr;
}

void CPC::Clock()
{
    if ((tick & 0x03) == 0)
    {
        switch(tick & 0x0F)
        {
        case 0:
            z80.WAIT = false;
            z80.Clock();
            crtc.Clock();
            gateArray.ProcessSync();
            gateArray.LoadVideoAddress();
            gateArray.ReadByte(true);
            z80.Clock2();
            fdc.Clock();
            tape.Clock();
            psg.Clock();
            break;
        case 4:
            gateArray.ReadByte(false);
            z80.WAIT = true;
            z80.Clock();
            z80.Clock2();
            fdc.Clock();
            break;
        case 8:
            z80.WAIT = false;
            z80.Clock();
            z80.Clock2();
            fdc.Clock();
            break;
        case 12:
            z80.WAIT = false;
            z80.Clock();
            z80.Clock2();
            fdc.Clock();
            break;
        }
    }
    gateArray.SetPixel();
    screen.Clock();
    tick++;
}

void CPC::Reset()
{
    tick = 0;
    z80.Reset();
    ppi.Reset();
    crtc.Reset();
    gateArray.Reset();
    keyboard.Reset();
    psg.Reset();
    fdc.Reset();
    screen.Init();
    SelectRAM(0);
    SelectROM(0);
}

BYTE CPC::PortRead(word addr)
{
    BYTE data = 0;
    if (!(addr & 0x4000))
        data = crtc.RD((addr & 0x0300) >> 8);
    else if (!(addr & 0x0800))
        data = ppi.RD((addr & 0x0300) >> 8);
    else if (!(addr & 0x0480))
    {
        if ((addr & 0x0100) != 0)
            data = (addr & 0x0001) ? fdc.RD_Data() : fdc.RD_State();
    }
    return data;
}

void CPC::AckInt() { gateArray.AckInt(); }

void CPC::PortWrite(word addr, BYTE value)
{
    if (!(addr & 0x8000)) gateArray.WR(value);
    if (!(addr & 0x4000)) crtc.WR((addr & 0x0300) >> 8, value);
    if (!(addr & 0x2000)) SelectROM(value);
    if (!(addr & 0x0800)) ppi.WR((addr & 0x0300) >> 8, value);
    if (!(addr & 0x0480))
    {
        if ((addr & 0x0100) == 0)
            fdc.SetMotor(value);
        else if ((addr & 0x0001) != 0)
            fdc.WR(value);
    }
}

BYTE CPC::GetRAMByteAt(word address)
{
    int bank = address >> 14;
    int addr = address & 0x3FFF;
    return RAM[bank][addr];
}

void CPC::UpdateMemoryMap()
{
    if (gateArray.LoROMActive)
        memPage[0] = (cartridgeEnabled && Cartridge) ? Cartridge : LoROM;
    else
        memPage[0] = RAM[0];

    memPage[1] = RAM[1];
    memPage[2] = RAM[2];

    if (gateArray.HiROMActive)
        memPage[3] = HiROM ? HiROM : zeroPage;
    else
        memPage[3] = RAM[3];
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
    UpdateMemoryMap();
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
    else
    {
        RAM[0] = RAMs[0]; RAM[1] = RAMs[1]; RAM[2] = RAMs[2]; RAM[3] = RAMs[3];
    }
    UpdateMemoryMap();
}

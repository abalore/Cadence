#include "CPC.h"
#include "GateArray.h"
#include "Keyboard.h"
#include "Z80.h"
#include "PPI.h"
#include "PSG.h"
#include "FDC.h"
#include "Tape.h"
#include "CRTScreen.h"
#include "Settings.h"
#include <QByteArray>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <cstring>

bool CPC::Breakpoint[65536];
std::string CPC::BreakpointCondition[65536];
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
BYTE CPC::selectedROM = 0;
BYTE *CPC::HiROMs[ROM_SLOTS];
BYTE *CPC::RAMs[36];
bool CPC::has512kExpansion = false;
BYTE *CPC::Cartridge;
bool CPC::cartridgeEnabled;
bool CPC::cartridgeDirty = false;
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
    if (filename[0] != '/')
    {
        QByteArray dir = Settings::CadenceDir().toLocal8Bit();
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir.constData(), filename);
    }
    else
        snprintf(fullpath, sizeof(fullpath), "%s", filename);
    FILE *file = fopen(fullpath, "rb");
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
    memset(Cartridge, 0, 524288);
    cartridgeDirty = false;
    FILE *file = fopen(filename, "rb");
    if (!file) return;

    unsigned char hdr[12];
    size_t hdrN = fread(hdr, 1, 12, file);
    bool isRiff = (hdrN == 12 &&
                   hdr[0] == 'R' && hdr[1] == 'I' && hdr[2] == 'F' && hdr[3] == 'F' &&
                   hdr[8] == 'A' && hdr[9] == 'M' && hdr[10] == 'S' && hdr[11] == '!');

    if (isRiff)
    {
        while (true)
        {
            char id[4];
            unsigned char szb[4];
            if (fread(id, 1, 4, file) != 4) break;
            if (fread(szb, 1, 4, file) != 4) break;
            unsigned int size = szb[0] | (szb[1] << 8) | (szb[2] << 16) | (szb[3] << 24);
            // CPR bank chunks are "cbNN" with decimal NN (00..31).
            if (id[0] == 'c' && id[1] == 'b' &&
                id[2] >= '0' && id[2] <= '9' && id[3] >= '0' && id[3] <= '9')
            {
                int bank = (id[2] - '0') * 10 + (id[3] - '0');
                if (bank >= 0 && bank < 32)
                {
                    unsigned int n = size < 0x4000u ? size : 0x4000u;
                    if (fread(Cartridge + bank * 0x4000, 1, n, file) != n) break;
                    if (size > n) fseek(file, size - n, SEEK_CUR);
                }
                else
                    fseek(file, size, SEEK_CUR);
            }
            else
                fseek(file, size, SEEK_CUR);
            if (size & 1u) fseek(file, 1, SEEK_CUR);  // RIFF 2-byte alignment
        }
    }
    else
    {
        fseek(file, 0, SEEK_SET);
        size_t n = fread(Cartridge, 1, 524288, file);
        (void)n;
    }
    fclose(file);
}

void CPC::SaveCartridge(const char *filename)
{
    if (Cartridge == nullptr) return;
    FILE *f = fopen(filename, "wb");
    if (!f) return;
    const unsigned int riffTotal = 4 + 32 * (8 + 0x4000);  // "AMS!" + 32 chunks
    fwrite("RIFF", 1, 4, f);
    fputc(riffTotal & 0xFF, f);
    fputc((riffTotal >> 8) & 0xFF, f);
    fputc((riffTotal >> 16) & 0xFF, f);
    fputc((riffTotal >> 24) & 0xFF, f);
    fwrite("AMS!", 1, 4, f);
    for (int bank = 0; bank < 32; bank++)
    {
        char id[4];
        // CPR bank chunks: decimal NN (00..31).
        id[0] = 'c'; id[1] = 'b';
        id[2] = char('0' + (bank / 10));
        id[3] = char('0' + (bank % 10));
        fwrite(id, 1, 4, f);
        fputc(0x00, f); fputc(0x40, f); fputc(0x00, f); fputc(0x00, f);
        fwrite(Cartridge + bank * 0x4000, 1, 0x4000, f);
    }
    fclose(f);
    cartridgeDirty = false;
}

void CPC::InsertBlankCartridge()
{
    if (Cartridge != nullptr) free(Cartridge);
    Cartridge = (BYTE *)malloc(524288);
    memset(Cartridge, 0, 524288);
    cartridgeDirty = false;
}

void CPC::Init()
{
    tick = 0;

    auto allocRAM = [](int count) {
        for (int i = 0; i < 36; i++) if (RAMs[i] != nullptr)
        {
            free(RAMs[i]);
            RAMs[i] = nullptr;
        }
        for (int i = 0; i < count; i++)
            RAMs[i] = (BYTE *) malloc(0x4000);
    };

    // With 512k expansion: always 36 pages (64k base + 8 × 64k blocks accessible via MMR bits 5-3).
    // On CPC6128, block 0 of the expansion coincides with the machine's built-in 64k.
    int basePages = (cpcType == CPCType::CPC6128) ? 8 : 4;
    int ramPages  = has512kExpansion ? 36 : basePages;

    switch(cpcType)
    {
    case CPCType::CPC464:
        ReadROM((char *)"ROM/BIOS_464.bin", -1);
        ReadROM((char *)"ROM/BASIC_464.bin", 0);
        break;
    case CPCType::CPC664:
        ReadROM((char *)"ROM/BIOS_664.bin", -1);
        ReadROM((char *)"ROM/BASIC_664.bin", 0);
        ReadROM((char *)"ROM/AMSDOS_6128.bin", 7);
        break;
    case CPCType::CPC6128:
        ReadROM((char *)"ROM/BIOS_6128.bin", -1);
        ReadROM((char *)"ROM/BASIC_6128.bin", 0);
        ReadROM((char *)"ROM/AMSDOS_6128.bin", 7);
        //ReadROM((char *)"ROM/PARADOS.ROM", 7);
        break;
    }
    allocRAM(ramPages);
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
    for (int i = 0; i < 36; i++) if (RAMs[i] != nullptr)
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
    selectedROM = number;
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
    if (cpcType == CPCType::CPC6128 || has512kExpansion)
    {
        // MMR bits 5-3 select the 64k block (0-7). Without 512k expansion only block 0 is valid.
        int block = has512kExpansion ? ((mmr >> 3) & 0x07) : 0;
        BYTE *exp[4];
        if (block == 0)
        {
            exp[0] = RAMs[4]; exp[1] = RAMs[5]; exp[2] = RAMs[6]; exp[3] = RAMs[7];
        }
        else
        {
            int base = 8 + (block - 1) * 4;
            exp[0] = RAMs[base + 0]; exp[1] = RAMs[base + 1];
            exp[2] = RAMs[base + 2]; exp[3] = RAMs[base + 3];
        }

        switch(mmr & 0x07)
        {
        case 0:
            RAM[0] = RAMs[0]; RAM[1] = RAMs[1]; RAM[2] = RAMs[2]; RAM[3] = RAMs[3];
            break;
        case 1:
            RAM[0] = RAMs[0]; RAM[1] = RAMs[1]; RAM[2] = RAMs[2]; RAM[3] = exp[3];
            break;
        case 2:
            RAM[0] = exp[0];  RAM[1] = exp[1];  RAM[2] = exp[2];  RAM[3] = exp[3];
            break;
        case 3:
            RAM[0] = RAMs[0]; RAM[1] = RAMs[3]; RAM[2] = RAMs[2]; RAM[3] = exp[3];
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            RAM[0] = RAMs[0]; RAM[1] = exp[(mmr & 0x07) - 4]; RAM[2] = RAMs[2]; RAM[3] = RAMs[3];
            break;
        }
    }
    else
    {
        RAM[0] = RAMs[0]; RAM[1] = RAMs[1]; RAM[2] = RAMs[2]; RAM[3] = RAMs[3];
    }
    UpdateMemoryMap();
}

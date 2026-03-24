#ifndef CPC_H
#define CPC_H

#include "defs.h"

#define ROM_SLOTS 16

enum CPCType
{
    CPC464,
    CPC664,
    CPC6128
};

class CPC
{
public:
    static void Init();
    static void Finalize();
    static void Clock();
    static void Reset();
    static BYTE GetByteAt(word address);
    static BYTE GetRAMByteAt(word address);
    static void SetByteAt(word address, BYTE b);
    static void SelectROM(BYTE number);
    static void ReadROM(char *filename, int number);
    static void ReadCartridge(char *filename);
    static void SelectRAM(BYTE mmr);
    static BYTE *RAM[4];
    static BYTE *LoROM;
    static BYTE *HiROM;
    static BYTE *HiROMs[ROM_SLOTS];
    static BYTE *RAMs[8];
    static BYTE bank;
    static word addr;
    static BYTE *Cartridge;
    static bool cartridgeEnabled;
    static CPCType cpcType;
    static BYTE tick;
    static BYTE baseVMA;

    static word AddressBUS;
    static BYTE DataBUS;

    static bool IORD;
    static bool IOWR;
    static bool MEMRD;
    static bool MEMWR;
    static bool INTACK;
    static bool MEMIO;
    static bool lastIORD;
    static bool lastIOWR;
    static bool lastMEMRD;
    static bool lastMEMWR;
    static bool lastINTACK;
    static bool lastMEMIO;
};


#endif // CPC_H

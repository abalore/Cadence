#ifndef CPC_H
#define CPC_H

#include "defs.h"
#include "CRTC.h"
#include "Keyboard.h"
#include "Tape.h"
#include "FDC.h"
#include "PSG.h"
#include "PPI.h"
#include "CRTScreen.h"
#include "GateArray.h"
#include "Z80.h"

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
    static BYTE GetRAMByteAt(word address);
    static void SetByteAt(word address, BYTE b);
    static void SelectROM(BYTE number);
    static void ReadROM(char *filename, int number);
    static void ClearROM(int number);
    static void ReadCartridge(char *filename);
    static void InsertBlankCartridge();
    static void SelectRAM(BYTE mmr);
    static void UpdateMemoryMap();
    static inline BYTE GetByteAt(word address) { return memPage[address >> 14][address & 0x3FFF]; }
    static BYTE PortRead(word addr);
    static void PortWrite(word addr, BYTE value);
    static void AckInt();
    static inline BYTE VRAMByte(dword addr) { return RAMs[(addr >> 14) & 3][addr & 0x3FFF]; }
    static BYTE *memPage[4];
    static BYTE *RAM[4];
    static BYTE *LoROM;
    static BYTE *HiROM;
    static BYTE *HiROMs[ROM_SLOTS];
    static BYTE *RAMs[8];
    static BYTE *Cartridge;
    static BYTE zeroPage[0x4000];
    static bool cartridgeEnabled;
    static CPCType cpcType;
    static BYTE tick;
    static BYTE baseVMA;
    static bool Breakpoint[65536];
    static CRTC crtc;
    static Keyboard keyboard;
    static Tape tape;
    static FDC fdc;
    static PSG psg;
    static PPI ppi;
    static CRTScreen screen;
    static GateArray gateArray;
    static Z80 z80;
};


#endif // CPC_H

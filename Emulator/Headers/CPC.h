#ifndef CPC_H
#define CPC_H

#include "defs.h"

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
    static BYTE *RAM[4];
    static BYTE *LoROM;
    static BYTE *HiROM;
    static BYTE *HiROMs[32];
    static BYTE *RAMs[8];
    static BYTE bank;
    static word addr;
};


#endif // CPC_H

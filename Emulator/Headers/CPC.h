#ifndef CPC_H
#define CPC_H

#include "defs.h"

class CPC
{
public:
    static void Init();
    static void Clock();
    static void Reset();
    static BYTE *ActiveROM();
    static BYTE *ActiveRAM();
    static BYTE *BaseRAM;
    static BYTE *LoROM;
    static BYTE *HiROM;
    static BYTE *HiROMs[32];
};


#endif // CPC_H

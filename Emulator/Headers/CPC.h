#ifndef CPC_H
#define CPC_H

#include "defs.h"
#include "ROM.h"
#include "RAM.h"
#include "PSG.h"

class CPC
{
public:
    static void Init();
    static void Clock();
    static void Reset();
    static ROM *ActiveROM();
    static word AddressBUS;
    static BYTE DataBUS;
    static RAM *InternalRAM;
    static ROM *LoROM;
    static ROM *HiROM;
    static ROM *ExpansionROM;
    static PSG *psg;
};


#endif // CPC_H

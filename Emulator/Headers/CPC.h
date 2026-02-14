#ifndef CPC_H
#define CPC_H

#include "defs.h"
#include "ROM.h"
#include "RAM.h"

class CPC
{
public:
    static void Init();
    static void Clock();
    static void Reset();
    static ROM *ActiveROM();
    static RAM *ActiveRAM();
    static word AddressBUS;
    static BYTE DataBUS;
    static RAM BaseRAM;
    static ROM LoROM;
    static ROM HiROM[32];
};


#endif // CPC_H

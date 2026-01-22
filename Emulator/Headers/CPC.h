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
    static ROM *ActiveROM();
    static BYTE bank();
    static word AddressBUS;
    static BYTE DataBUS;
    static RAM *InternalRAM;
    static ROM *LoROM;
    static ROM *HiROM;
};


#endif // CPC_H

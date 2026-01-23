#ifndef PSG_H
#define PSG_H

#include "defs.h"

class PSG
{
public:
    static void Init();
    static void Clock();
    static BYTE ReadData();
    static void WriteData(BYTE data);
    static BYTE PortA;
    static bool BC1;
    static bool BDIR;
private:
    static BYTE inputRegister;
    static BYTE outputRegister;
    static BYTE selectedRegister;
    static BYTE registers[16];
};

#endif // PSG_H

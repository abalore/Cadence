#ifndef EMULATOR_H
#define EMULATOR_H

#include "defs.h"

class Emulator
{
public:
    static void Init();
    static void Clock();
    static void Reset();
    static void ReadROM(char *filename, BYTE *dest);
private:
    static word tapeTick;
};

#endif // EMULATOR_H

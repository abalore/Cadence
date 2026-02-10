#ifndef ROMSELECTOR_H
#define ROMSELECTOR_H

#include "defs.h"

class ROMSelector
{
public:
    static void Init();
    static void Clock_IO_WR();
    static BYTE SelectedROM;
};

#endif // ROMSELECTOR_H

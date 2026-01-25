#ifndef ROMSELECTOR_H
#define ROMSELECTOR_H

#include "defs.h"

class ROMSelector
{
public:
    static void Init();
    static void IOClock();
    static BYTE SelectedROM;
};

#endif // ROMSELECTOR_H

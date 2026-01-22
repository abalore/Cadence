#ifndef RAM_H
#define RAM_H

#include "defs.h"

class RAM
{
public:
    RAM();
    void Clock();
    BYTE *MEM;
};

#endif // RAM_H

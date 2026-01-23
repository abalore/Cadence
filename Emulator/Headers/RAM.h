#ifndef RAM_H
#define RAM_H

#include "defs.h"

class RAM
{
public:
    RAM();
    void Clock();
    BYTE MEM[65536];
};

#endif // RAM_H

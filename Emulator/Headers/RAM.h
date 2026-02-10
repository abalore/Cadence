#ifndef RAM_H
#define RAM_H

#include "defs.h"

class RAM
{
public:
    RAM();
    void Clock_RD();
    void Clock_WR();
    BYTE MEM[65536];
};

#endif // RAM_H

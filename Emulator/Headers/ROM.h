#ifndef ROM_H
#define ROM_H

#include "defs.h"

class ROM
{
public:
    ROM(word location);
    void Clock_RD();
    BYTE *MEM;
private:
    word Location;
};

#endif // ROM_H

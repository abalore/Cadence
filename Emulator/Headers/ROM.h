#ifndef ROM_H
#define ROM_H

#include "defs.h"

class ROM
{
public:
    ROM(BYTE number);
    void Clock_RD();
    BYTE *MEM;
private:
    word Location;
    BYTE Number;
};

#endif // ROM_H

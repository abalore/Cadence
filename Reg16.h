#ifndef REG16_H
#define REG16_H

#include "defs.h"

class Reg16
{
public:
    Reg16(BYTE *h, BYTE *l);
    inline word Get() { return (*H << 8) | *L; }
    inline void Set(word value) { *H = value >> 8; *L = value & 0xFF; }
    BYTE *H;
    BYTE *L;
};

#endif // REG16_H

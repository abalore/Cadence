#ifndef REG16_H
#define REG16_H

#include "defs.h"

class Reg16
{
public:
    Reg16(BYTE *h, BYTE *l);
    void Set(word value);
    word Get();
    BYTE *H;
    BYTE *L;
};

#endif // REG16_H

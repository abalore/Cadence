#ifndef FLAG_H
#define FLAG_H

#include "defs.h"

class Flag
{
public:
    Flag(BYTE *s, BYTE m);
    void Set(bool value);
    bool Get();
private:
    BYTE *storage;
    BYTE mask;
    BYTE nmask;
};

#endif // FLAG_H

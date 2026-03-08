#ifndef COUNTER_H
#define COUNTER_H

#include "Headers/defs.h"

class Counter
{
public:
    Counter(BYTE mask);
    void Step();
    void Reset();
    BYTE value;
private:
    BYTE mask;
};

#endif // COUNTER_H

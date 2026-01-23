#ifndef PPI_H
#define PPI_H

#include "defs.h"

class PPI
{
public:
    static void Init();
    static void IOClock();
private:
    static BYTE controlWord;
};

#endif // PPI_H

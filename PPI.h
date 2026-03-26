#ifndef PPI_H
#define PPI_H

#include "defs.h"

class PPI
{
public:
    static void Reset();
    static BYTE RD(BYTE reg);
    static void WR(BYTE reg, BYTE value);
    static BYTE controlWord;
private:
    static void ApplyLC();
    static void ApplyHC();
    static BYTE aMode;
    static BYTE bMode;
    static BYTE aHandshake;
    static BYTE bHandshake;
    static bool lCIO;
    static bool hCIO;
    static bool aIO;
    static bool bIO;
    static BYTE lC;
    static BYTE hC;

};

#endif // PPI_H

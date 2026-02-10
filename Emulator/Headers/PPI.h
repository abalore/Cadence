#ifndef PPI_H
#define PPI_H

#include "defs.h"

class PPI
{
public:
    static void Init();
    static void Clock_IO_RD();
    static void Clock_IO_WR();
    static BYTE controlWord;
    static BYTE PortA;
    static BYTE PortB;
    static BYTE PortC;
private:
    static void ApplyLC();
    static void ApplyHC();
    static BYTE DataBuffer;
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

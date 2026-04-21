#ifndef PPI_H
#define PPI_H

#include "defs.h"

class PPI
{
public:
    void Reset();
    BYTE RD(BYTE reg);
    void WR(BYTE reg, BYTE value);
    BYTE controlWord;
private:
    void UpdatePortC_Low();
    void UpdatePortC_High();
    BYTE aMode;
    BYTE bMode;
    BYTE aHandshake;
    BYTE bHandshake;
    bool lCIO;
    bool hCIO;
    bool aIO;
    bool bIO;
    BYTE lC;
    BYTE hC;
};

#endif // PPI_H

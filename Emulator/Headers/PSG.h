#ifndef PSG_H
#define PSG_H

#include "defs.h"

class PSG
{
public:
    void Init();
    void Clock();
    BYTE ReadData();
    void WriteData(BYTE data);
    BYTE PortA;
    bool BC1;
    bool BDIR;
    BYTE outputA;
    BYTE outputB;
    BYTE outputC;
    volatile bool DataAvailable;
    volatile BYTE output;
private:
    BYTE inputRegister;
    BYTE outputRegister;
    BYTE selectedRegister;
    BYTE registers[16];
    word counterA;
    word counterB;
    word counterC;
    BYTE divider;
    bool bitA;
    bool bitB;
    bool bitC;
    word periodA;
    word periodB;
    word periodC;
    bool mixA;
    bool mixB;
    bool mixC;
};

#endif // PSG_H


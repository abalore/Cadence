#ifndef PSG_H
#define PSG_H

#include "defs.h"

#define PSG_BUFFER_SIZE 1500

class PSG
{
public:
    static void Init();
    static void Clock();
    static BYTE ReadData();
    static void WriteData(BYTE data);
    static BYTE PortA;
    static bool BC1;
    static bool BDIR;
    static BYTE outputA;
    static BYTE outputB;
    static BYTE outputC;
    static BYTE buffer[PSG_BUFFER_SIZE];
    static int bufferIndex;
private:
    static void UpdateEnvelope();
    static void UpdateNoise();
    static BYTE GetCurrentEnvelopeLevel();
    static BYTE inputRegister;
    static BYTE outputRegister;
    static BYTE selectedRegister;
    static BYTE registers[16];
    static word counterA;
    static word counterB;
    static word counterC;
    static BYTE divider;
    static BYTE envelopeDivider;
    static BYTE envelopeStage;
    static BYTE envelopeLevel;
    static bool envelopeDir;
    static word envelopePeriod;
    static BYTE noiseDivider;
    static bool noiseLevel;
    static bool bitA;
    static bool bitB;
    static bool bitC;
    static word periodA;
    static word periodB;
    static word periodC;
    static bool mixA;
    static bool mixB;
    static bool mixC;
    static bool noiseA;
    static bool noiseB;
    static bool noiseC;
    static BYTE tVol;
};

#endif // PSG_H


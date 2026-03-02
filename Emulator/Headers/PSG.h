#ifndef PSG_H
#define PSG_H

#include "defs.h"

#define PSG_BUFFER_SIZE 1500

enum EnvelopeDir
{
    EDUp,
    EDDown
};

class PSG
{
public:
    static void Reset();
    static void Clock();
    static void SelectFunction(bool bdir, bool bc1);
    static BYTE ReadData();
    static void WriteData(BYTE data);
    static BYTE outputA;
    static BYTE outputB;
    static BYTE outputC;
    static BYTE buffer[PSG_BUFFER_SIZE];
    static int bufferIndex;
private:
    static void UpdateEnvelope();
    static void UpdateNoise();
    static BYTE GetCurrentEnvelopeLevel();
    static void ApplyChange();
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
    static EnvelopeDir envelopeDir;
    static bool envelopeContinue;
    static bool envelopeAttack;
    static bool envelopeHold;
    static bool envelopeAlternate;
    static word envelopePeriod;
    static word envelopeCounter;
    static bool envelopeRunning;
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
    static BYTE tVolA;
    static BYTE tVolB;
    static BYTE tVolC;
    static bool BC1;
    static bool BDIR;
    constexpr static const BYTE volumes[16]
    {
        0,
        0,
        0,
        1,
        2,
        4,
        6,
        8,
        10,
        13,
        16,
        20,
        24,
        28,
        32,
        37
    };
};

#endif // PSG_H


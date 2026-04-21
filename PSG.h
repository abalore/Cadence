#ifndef PSG_H
#define PSG_H

#include "defs.h"
#include <cstdint>

#define PSG_BUFFER_SIZE 1500

enum EnvelopeDir
{
    EDUp,
    EDDown
};

class PSG
{
public:
    void Reset();
    void Clock();
    void SelectFunction(bool bdir, bool bc1);
    BYTE ReadData();
    void WriteData(BYTE data);
    BYTE outputA;
    BYTE outputB;
    BYTE outputC;
    BYTE buffer[PSG_BUFFER_SIZE];
    int bufferIndex;
private:
    void UpdateEnvelope();
    void UpdateNoise();
    BYTE GetCurrentEnvelopeLevel();
    void ApplyChangeFromControl();
    void ApplyChangeFromData();
    BYTE inputRegister;
    BYTE outputRegister;
    BYTE selectedRegister;
    BYTE registers[16];
    word counterA;
    word counterB;
    word counterC;
    BYTE divider;
    BYTE envelopeDivider;
    BYTE envelopeStage;
    BYTE envelopeLevel;
    EnvelopeDir envelopeDir;
    bool envelopeContinue;
    bool envelopeAttack;
    bool envelopeHold;
    bool envelopeAlternate;
    word envelopePeriod;
    word envelopeCounter;
    bool envelopeRunning;
    BYTE noiseDivider;
    bool noiseLevel;
    uint32_t noiseLFSR;
    bool bitA;
    bool bitB;
    bool bitC;
    word periodA;
    word periodB;
    word periodC;
    bool mixA;
    bool mixB;
    bool mixC;
    bool noiseA;
    bool noiseB;
    bool noiseC;
    BYTE tVolA;
    BYTE tVolB;
    BYTE tVolC;
    bool BC1;
    bool BDIR;
    BYTE latch;
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

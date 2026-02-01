#ifndef TAPE_H
#define TAPE_H

#include "defs.h"
#include <pulse/simple.h>

enum TapeSource
{
    None,
    WAV,
    Input,
    CDT
};

class Tape
{
public:
    static void LoadWAV( char *filename);
    static void Clock();
    static void SetMotorState(bool state);
    static void FromAudioInput(bool value);
    static BYTE GetLevel();
private:
    static void FreeBuffer();
    static TapeSource tapeSource;
    static BYTE *buffer;
    static unsigned long bufferReadIndex;
    static unsigned long bufferWriteIndex;
    static unsigned long bufferSize;
    static bool motorState;
    static bool lastLevel;
    static BYTE level;
    static pa_simple *pa;
};

#endif // TAPE_H

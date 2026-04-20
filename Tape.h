#ifndef TAPE_H
#define TAPE_H

#include "defs.h"
#include "CDT.h"

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
    static void LoadCDT( char *filename);
    static void Eject();
    static void Clock();
    static void SetMotorState(bool state);
    static BYTE GetLevel();
    static volatile bool motorState;
    static volatile bool audioEnabled;
private:
    static void FreeBuffer();
    static TapeSource tapeSource;
    static BYTE *buffer;
    static unsigned long bufferReadIndex;
    static unsigned long bufferWriteIndex;
    static unsigned long bufferSize;
    static bool lastLevel;
    static BYTE level;
    static class CDT cdt;
    static word tapeTick;
};

#endif // TAPE_H

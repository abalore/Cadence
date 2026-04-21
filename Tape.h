#ifndef TAPE_H
#define TAPE_H

#include "defs.h"
#include "CDT.h"
#include <atomic>

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
    ~Tape() { FreeBuffer(); }
    void LoadWAV(char *filename);
    void LoadCDT(char *filename);
    void Eject();
    void Clock();
    void SetMotorState(bool state);
    BYTE GetLevel();
    std::atomic<bool> motorState{false};
    std::atomic<bool> audioEnabled{true};
private:
    void FreeBuffer();
    TapeSource tapeSource = TapeSource::None;
    BYTE *buffer = nullptr;
    unsigned long bufferReadIndex = 0;
    unsigned long bufferWriteIndex = 0;
    unsigned long bufferSize = 0;
    bool lastLevel = false;
    BYTE level = 0;
    class CDT cdt;
    word tapeTick = 0;
};

#endif // TAPE_H

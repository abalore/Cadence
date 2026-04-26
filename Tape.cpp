#include "Tape.h"
#include <stdio.h>
#include <stdlib.h>
#include <QFileInfo>

void Tape::LoadWAV(char *filename)
{
    FreeBuffer();
    qint64 sz = QFileInfo(filename).size();
    if (sz <= 0) return;
    FILE *f = fopen(filename, "rb");
    if (!f) return;
    buffer = (BYTE *)malloc(sz);
    bufferSize = sz;
    size_t n = fread(buffer, 1, bufferSize, f);
    fclose(f);
    if (n != bufferSize) { FreeBuffer(); return; }
    bufferReadIndex = 44;
    tapeSource = TapeSource::WAV;
}

void Tape::LoadCDT(char *filename)
{
    FreeBuffer();
    qint64 sz = QFileInfo(filename).size();
    if (sz <= 0) return;
    FILE *f = fopen(filename, "rb");
    if (!f) return;
    buffer = (BYTE *)malloc(sz);
    bufferSize = sz;
    size_t n = fread(buffer, 1, bufferSize, f);
    fclose(f);
    if (n != bufferSize) { FreeBuffer(); return; }
    if (cdt.Init(buffer, bufferSize))
    {
        bufferReadIndex = 44;
        tapeSource = TapeSource::CDT;
    }
    else
        FreeBuffer();
}

void Tape::Clock()
{
    tapeTick++;
    if (tapeTick >= 20) // (1 Mhz / sample rate) i.e. 20 for 50 Khz
    {
        tapeTick = 0;
        switch(tapeSource)
        {
        case TapeSource::None:
            break;
        case TapeSource::WAV:
            if (motorState)
            {

                if (buffer != 0)
                {
                    level = lastLevel;
                    if (bufferReadIndex < bufferSize)
                    {
                        if (buffer[bufferReadIndex] > 0xA0)
                            level = true;
                        if (buffer[bufferReadIndex++] < 0x60)
                            level = false;
                    }
                    else
                    {
                        tapeSource = TapeSource::None;
                        FreeBuffer();
                    }
                    lastLevel = level;
                }

            }
            break;
        case TapeSource::Input:
            break;
        case TapeSource::CDT:
            if (motorState)
            {
                if (cdt.EndOfFile)
                    tapeSource = TapeSource::None;
                else
                    level = cdt.GetNextLevel();
            }
            break;
        }
    }
}

int Tape::GetProgressPercent() const
{
    if (tapeSource == TapeSource::WAV)
    {
        if (bufferSize <= 44) return 0;
        return int((bufferReadIndex - 44) * 100 / (bufferSize - 44));
    }
    if (tapeSource == TapeSource::CDT)
    {
        if (bufferSize == 0) return 0;
        return int(cdt.GetReadOffset() * 100 / bufferSize);
    }
    return 0;
}

void Tape::Rewind()
{
    switch (tapeSource)
    {
    case TapeSource::WAV:
        bufferReadIndex = 44;
        lastLevel = false;
        level = 0;
        break;
    case TapeSource::CDT:
        if (buffer && bufferSize)
            cdt.Init(buffer, bufferSize);
        break;
    default:
        break;
    }
}

void Tape::SetMotorState(bool state)
{
    if (state != motorState)
        motorState = state;
}

BYTE Tape::GetLevel()
{
    return level;
}

void Tape::Eject()
{
    FreeBuffer();
    tapeSource = TapeSource::None;
}

void Tape::FreeBuffer()
{
    if (buffer != 0)
    {
        free(buffer);
        buffer = 0;
    }
}

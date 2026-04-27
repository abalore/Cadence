#include "Tape.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <QFileInfo>

void Tape::LoadWAV(char *filename)
{
    FreeBuffer();
    qint64 sz = QFileInfo(filename).size();
    if (sz < 44) return;
    FILE *f = fopen(filename, "rb");
    if (!f) return;
    buffer = (BYTE *)malloc(sz);
    bufferSize = sz;
    size_t n = fread(buffer, 1, bufferSize, f);
    fclose(f);
    if (n != bufferSize) { FreeBuffer(); return; }

    if (memcmp(buffer, "RIFF", 4) != 0 || memcmp(buffer + 8, "WAVE", 4) != 0)
    { FreeBuffer(); return; }

    int sampleRate = 50000, bitsPerSample = 8, channels = 1;
    unsigned long dataStart = 0, dataLen = 0;
    unsigned long p = 12;
    while (p + 8 <= bufferSize)
    {
        const unsigned char *id = buffer + p;
        unsigned long len = (unsigned long)buffer[p+4] | ((unsigned long)buffer[p+5] << 8) | ((unsigned long)buffer[p+6] << 16) | ((unsigned long)buffer[p+7] << 24);
        if (memcmp(id, "fmt ", 4) == 0 && p + 8 + 16 <= bufferSize)
        {
            channels      = buffer[p+10] | (buffer[p+11] << 8);
            sampleRate    = buffer[p+12] | (buffer[p+13] << 8) | (buffer[p+14] << 16) | (buffer[p+15] << 24);
            bitsPerSample = buffer[p+22] | (buffer[p+23] << 8);
        }
        else if (memcmp(id, "data", 4) == 0)
        {
            dataStart = p + 8;
            dataLen = len;
            break;
        }
        p += 8 + len + (len & 1);
    }
    if (dataStart == 0 || sampleRate <= 0)
    { FreeBuffer(); return; }

    bufferReadIndex = dataStart;
    wavDataStart = dataStart;
    if (dataStart + dataLen <= bufferSize)
        bufferSize = dataStart + dataLen;
    wavBytesPerFrame = (bitsPerSample / 8) * channels;
    if (wavBytesPerFrame < 1) wavBytesPerFrame = 1;
    wavIs16Bit = (bitsPerSample == 16);
    wavTickDiv = 1000000 / sampleRate;
    if (wavTickDiv < 1) wavTickDiv = 1;
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
    int divisor = (tapeSource == TapeSource::WAV) ? wavTickDiv : 20;
    if (tapeTick >= divisor)
    {
        tapeTick = 0;
        switch(tapeSource)
        {
        case TapeSource::None:
            break;
        case TapeSource::WAV:
            if (motorState && buffer)
            {
                level = lastLevel;
                if (bufferReadIndex + (unsigned long)wavBytesPerFrame <= bufferSize)
                {
                    if (wavIs16Bit)
                    {
                        int16_t s = (int16_t)(buffer[bufferReadIndex] | (buffer[bufferReadIndex + 1] << 8));
                        if (s >  0x2000) level = true;
                        else if (s < -0x2000) level = false;
                    }
                    else
                    {
                        BYTE b = buffer[bufferReadIndex];
                        if (b > 0xA0) level = true;
                        else if (b < 0x60) level = false;
                    }
                    bufferReadIndex += wavBytesPerFrame;
                }
                else
                {
                    tapeSource = TapeSource::None;
                    FreeBuffer();
                }
                lastLevel = level;
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
        if (bufferSize <= wavDataStart) return 0;
        return int((bufferReadIndex - wavDataStart) * 100 / (bufferSize - wavDataStart));
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
        bufferReadIndex = wavDataStart;
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

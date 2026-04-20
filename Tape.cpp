#include "Tape.h"
#include <stdio.h>
#include <stdlib.h>
#include <filesystem>

using namespace std;

BYTE Tape::level = 0;
BYTE *Tape::buffer = 0;
unsigned long Tape::bufferReadIndex;
unsigned long Tape::bufferWriteIndex;
unsigned long Tape::bufferSize;
volatile bool Tape::motorState = false;
volatile bool Tape::audioEnabled = true;
bool Tape::lastLevel = false;
TapeSource Tape::tapeSource = TapeSource::None;
class CDT Tape::cdt;
word Tape::tapeTick = 0;

void Tape::LoadWAV(char *filename)
{
    FreeBuffer();
    bufferSize = filesystem::file_size(filename);
    buffer = (BYTE *)malloc(bufferSize);
    FILE *f = fopen(filename, "r");
    size_t n = fread(buffer, 1, bufferSize, f);
    (void)n;
    fclose(f);
    bufferReadIndex = 44;
    tapeSource = TapeSource::WAV;
}

void Tape::LoadCDT(char *filename)
{
    FreeBuffer();
    bufferSize = filesystem::file_size(filename);
    buffer = (BYTE *)malloc(bufferSize);
    FILE *f = fopen(filename, "r");
    size_t n = fread(buffer, 1, bufferSize, f);
    (void)n;
    fclose(f);
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

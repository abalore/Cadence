#include "Headers/Tape.h"
#include <stdio.h>
#include <stdlib.h>
#include <filesystem>

using namespace std;

BYTE Tape::level = 0;
BYTE *Tape::buffer = 0;
unsigned long Tape::bufferReadIndex;
unsigned long Tape::bufferWriteIndex;
unsigned long Tape::bufferSize;
bool Tape::motorState = false;
bool Tape::lastLevel = false;
TapeSource Tape::tapeSource = TapeSource::None;
pa_simple *Tape::pa;

void Tape::LoadWAV(char *filename)
{
    FreeBuffer();
    bufferSize = filesystem::file_size(filename);
    buffer = (BYTE *)malloc(bufferSize);
    FILE *f = fopen(filename, "r");
    fread(buffer, 1, bufferSize, f);
    fclose(f);
    bufferReadIndex = 44;
    tapeSource = TapeSource::WAV;
}

void Tape::Clock()
{
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
        level = lastLevel;
        bufferReadIndex++;
        //if (bufferReadIndex == bufferSize)
        //{
            pa_simple_read(pa, buffer, 3, NULL);
            bufferReadIndex = 0;
        //}
        if (buffer[1] > 0xA0)
            level = true;
        if (buffer[1] < 0x60)
            level = false;
        lastLevel = level;
        break;
    case TapeSource::CDT:
        break;
    }
}

void Tape::SetMotorState(bool state)
{
    motorState = state;
}

BYTE Tape::GetLevel()
{
    return level;
}

void Tape::FromAudioInput(bool value)
{
    FreeBuffer();
    if (value)
    {
        buffer = (BYTE *)malloc(10000);
        tapeSource = TapeSource::Input;
        pa_sample_spec ss;
        ss.format = PA_SAMPLE_U8;
        ss.channels = 1;
        ss.rate = 62500;
        pa = pa_simple_new(NULL, "AAE", PA_STREAM_RECORD, NULL, "Emulator", &ss, NULL, NULL, NULL);
        bufferReadIndex = 0;
        bufferWriteIndex = 5000;
        bufferSize = 100;
        //pa_simple_read(pa, buffer, 10000, NULL);

    }
    else
    {
        tapeSource = TapeSource::None;
        pa_simple_free(pa);
    }
}

void Tape::FreeBuffer()
{
    if (buffer != 0)
    {
        free(buffer);
        buffer = 0;
    }
}

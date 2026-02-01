#include "Headers/PSG.h"
#include "Headers/Keyboard.h"
#include "Headers/Tape.h"
#include <stdlib.h>

BYTE PSG::PortA;
bool PSG::BC1;
bool PSG::BDIR;
BYTE PSG::outputA;
BYTE PSG::outputB;
BYTE PSG::outputC;
BYTE PSG::buffer[PSG_BUFFER_SIZE];
int PSG::bufferIndex;
BYTE PSG::inputRegister;
BYTE PSG::outputRegister;
BYTE PSG::selectedRegister;
BYTE PSG::registers[16];
word PSG::counterA;
word PSG::counterB;
word PSG::counterC;
BYTE PSG::divider;
BYTE PSG::envelopeDivider;
BYTE PSG::envelopeStage;
BYTE PSG::envelopeLevel;
bool PSG::envelopeDir;
word PSG::envelopePeriod;
BYTE PSG::noiseDivider;
bool PSG::noiseLevel;
bool PSG::bitA;
bool PSG::bitB;
bool PSG::bitC;
word PSG::periodA;
word PSG::periodB;
word PSG::periodC;
bool PSG::mixA;
bool PSG::mixB;
bool PSG::mixC;
bool PSG::noiseA;
bool PSG::noiseB;
bool PSG::noiseC;
BYTE PSG::tVol;


void PSG::Init()
{
    BDIR = false;
    BC1 = true;
    counterA = 0;
    counterB = 0;
    counterC = 0;
    outputA = 0;
    outputB = 0;
    outputC = 0;
    divider = 0;
    envelopeDivider = 0;
    envelopeStage = 0;
    envelopeLevel = 0;
    envelopeDir = true;
    envelopePeriod = 0;
    noiseDivider = 0;
    noiseLevel = 0;

    bitA = false;
    bitB = false;
    bitC = false;
    periodA = 0;
    periodB = 0;
    periodC = 0;
    mixA = false;
    mixB = false;
    mixC = false;
    noiseA = false;
    noiseB = false;
    noiseC = false;
    bufferIndex = 0;
}

void PSG::Clock()
{
    if (BDIR)
    {
        if (BC1)
            selectedRegister = inputRegister;
        else
        {
            registers[selectedRegister] = inputRegister;
            switch (selectedRegister)
            {
            case 0:
            case 1:
                periodA = ((registers[1] & 0x0F ) * 256 + registers[0]) >> 1;
                break;
            case 2:
            case 3:
                periodB = ((registers[3] & 0x0F ) * 256 + registers[2]) >> 1;
                break;
            case 4:
            case 5:
                periodC = ((registers[5] & 0x0F ) * 256 + registers[4]) >> 1;
                break;
            case 7:
                mixA = !(inputRegister & 0x01);
                mixB = !(inputRegister & 0x02);
                mixC = !(inputRegister & 0x04);
                noiseA = !(inputRegister & 0x08);
                noiseB = !(inputRegister & 0x10);
                noiseC = !(inputRegister & 0x20);
                break;
            case 8:
            case 9:
            case 10:
                if (inputRegister == 16)
                {
                    tVol = 10;
                }
                break;
            case 11:
            case 12:
                envelopePeriod = registers[12] * 256 + registers[11];
                break;
            case 13:
            case 14:
                noiseDivider = (registers[6] & 0x1F);
                break;
            }
        }
    }
    else
    {
        if (BC1)
        {
            if (selectedRegister < 0x0E)
            {
                outputRegister = registers[selectedRegister];
            }
            else if (selectedRegister == 0x0E)
                outputRegister = Keyboard::Read();
        }
        else
            outputRegister = 0xFF;
    }
    if (envelopeDivider == 0)
        UpdateEnvelope();
    envelopeDivider++;
    divider++;
    if (divider == 16)
    {
        UpdateNoise();
        divider = 0;
        counterA++;
        if (counterA >= periodA)
        {
            counterA = 0;
            bitA = !bitA;
        }
        if (mixA)
        {
            tVol = registers[8];
            if (tVol & 0x10)
                tVol = GetCurrentEnvelopeLevel();
            outputA = tVol * bitA;
        }
        else outputA = 0;
        counterB++;
        if (counterB >= periodB)
        {
            counterB = 0;
            bitB = !bitB;
        }
        if (mixB)
        {
            tVol = registers[9];
            if (tVol & 0x10)
                tVol = GetCurrentEnvelopeLevel();
            outputB = tVol * bitB;
        }
        else outputB = 0;
        counterC++;
        if (counterC >= periodC)
        {
            counterC = 0;
            bitC = !bitC;
        }
        if (mixC)
        {
            tVol = registers[10];
            if (tVol & 0x10)
                tVol = GetCurrentEnvelopeLevel();
            outputC = tVol * bitC;
        }
        else outputC = 0;
        if (noiseA)
            outputA += registers[8] * noiseLevel;
        if (noiseB)
            outputB += registers[9] * noiseLevel;
        if (noiseC)
            outputC += registers[10] * noiseLevel;

        buffer[bufferIndex] = outputA + outputB + outputC + Tape::GetLevel() * 7;
        if (bufferIndex < PSG_BUFFER_SIZE)
            bufferIndex++;
    }
}

BYTE PSG::ReadData()
{
    return outputRegister;
}

void PSG::WriteData(BYTE data)
{
    inputRegister = data;
}

void PSG::UpdateEnvelope()
{
    envelopePeriod--;
    if (envelopePeriod == 0)
    {
        envelopePeriod = registers[12] * 256 + registers[11];
        tVol = envelopeLevel;
        envelopeLevel += envelopeDir ? 1 : 0xFF;
        envelopeStage++;
        if (envelopeStage == 16)
        {
            if (registers[13] & 0x01)
            {
                envelopeStage--;
                envelopeLevel = tVol;
            }
            else
            {
                envelopeStage = 0;
                if (registers[13] & 0x02)
                    envelopeDir = !envelopeDir;
                else
                    envelopeDir = registers[13] & 0x04;
                envelopeLevel = envelopeDir ? 0 : 15;
            }
        }
    }
}

BYTE PSG::GetCurrentEnvelopeLevel()
{
    return envelopeLevel;
}

void PSG::UpdateNoise()
{
    noiseDivider--;
    if (noiseDivider == 0)
    {
        noiseDivider = (registers[6] & 0x1F);
        noiseLevel = random() < (RAND_MAX / 2);
    }
}

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
EnvelopeDir PSG::envelopeDir;
bool PSG::envelopeAttack;
bool PSG::envelopeContinue;
bool PSG::envelopeHold;
bool PSG::envelopeAlternate;
word PSG::envelopePeriod;
word PSG::envelopeCounter;
bool PSG::envelopeRunning;
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
BYTE PSG::tVolA;
BYTE PSG::tVolB;
BYTE PSG::tVolC;


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
    envelopeDir = EnvelopeDir::EDNone;
    envelopePeriod = 0;
    envelopeRunning = false;
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
    if (envelopeRunning)
    {
        if (envelopeDivider == 16)
        {
            UpdateEnvelope();
            envelopeDivider = 0;
        }
        else
            envelopeDivider++;
    }

    divider++;
    if (divider == 16)
    {
        divider = 0;
        UpdateNoise();

        counterA++; if (counterA >= periodA) { counterA = 0; bitA = !bitA; }
        counterB++; if (counterB >= periodB) { counterB = 0; bitB = !bitB; }
        counterC++; if (counterC >= periodC) { counterC = 0; bitC = !bitC; }

        outputA = mixA && bitA;
        outputB = mixB && bitB;
        outputC = mixC && bitC;

        if (noiseA)
            outputA |= noiseLevel;
        if (noiseB)
            outputB |= noiseLevel;
        if (noiseC)
            outputC |= noiseLevel;

        outputA *= volumes[tVolA & 0x10 ? envelopeLevel : tVolA];
        outputB *= volumes[tVolB & 0x10 ? envelopeLevel : tVolB];
        outputC *= volumes[tVolC & 0x10 ? envelopeLevel : tVolC];

        if (bufferIndex < PSG_BUFFER_SIZE)
        {
            buffer[bufferIndex] = (outputA + outputB + outputC + Tape::GetLevel() * 10);
            bufferIndex++;
        }
    }
}

void PSG::SelectFunction(bool bdir, bool bc1)
{
    BDIR = bdir;
    BC1 = bc1;
    ApplyChange();
}

void PSG::ApplyChange()
{
    if (BC1 == true && BDIR == true)
        selectedRegister = inputRegister;
    else if (BC1 == false && BDIR == true)
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
        case 6:
            noiseDivider = (registers[6] & 0x1F);
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
            tVolA = registers[8];
            break;
        case 9:
            tVolB = registers[9];
            break;
        case 10:
            tVolC = registers[10];
            break;
        case 11:
        case 12:
            envelopePeriod = registers[12] * 256 + registers[11];
            envelopeCounter = envelopePeriod;
            break;
        case 13:
            envelopeContinue = registers[13] & 0x08;
            envelopeAttack = registers[13] & 0x04;
            envelopeAlternate = registers[13] & 0x02;
            envelopeHold = registers[13] & 0x01;
            envelopeCounter = envelopePeriod;
            envelopeRunning = true;
            if (envelopeAttack)
            {
                envelopeLevel = 0;
                envelopeDir = EnvelopeDir::EDUp;
            }
            else
            {
                envelopeLevel = 15;
                envelopeDir = EnvelopeDir::EDDown;
            }
            break;
        }
    }
    else if (BC1 == true && BDIR == false)
    {
        if (selectedRegister < 0x0E)
            outputRegister = registers[selectedRegister];
        else if (selectedRegister == 0x0E)
            outputRegister = Keyboard::Read();
    }
}

BYTE PSG::ReadData()
{
    return outputRegister;
}

void PSG::WriteData(BYTE data)
{
    inputRegister = data;
    ApplyChange();
}

void PSG::UpdateEnvelope()
{
    envelopeCounter--;
    if (envelopeCounter == 0)
    {
        envelopeCounter = envelopePeriod;
        switch(envelopeDir)
        {
        case EnvelopeDir::EDNone:
            break;
        case EnvelopeDir::EDUp:
            envelopeLevel++;
            if (envelopeLevel >= 15)
            {
                if (!envelopeContinue)
                {
                    envelopeRunning = false;
                    envelopeLevel = 0;
                }
                else
                {
                    if (envelopeHold)
                    {
                        envelopeRunning = false;
                        if (envelopeAlternate)
                            envelopeLevel = 0;
                    }
                    else
                    {
                        if (envelopeAlternate)
                            envelopeDir = EnvelopeDir::EDDown;
                        else
                            envelopeLevel = 0;
                    }
                }
            }
            break;
        case EnvelopeDir::EDDown:
            envelopeLevel--;
            if (envelopeLevel == 0)
            {
                if (!envelopeContinue)
                {
                    envelopeRunning = false;
                    envelopeLevel = 0;
                }
                else
                {
                    if (envelopeHold)
                    {
                        envelopeRunning = false;
                        if (envelopeAlternate)
                            envelopeLevel = 15;
                    }
                    else
                    {
                        if (envelopeAlternate)
                            envelopeDir = EnvelopeDir::EDUp;
                        else
                            envelopeLevel = 15;
                    }
                }
            }
            break;
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

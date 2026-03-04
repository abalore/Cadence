#include "Headers/CRTScreen.h"
#include "Headers/GateArray.h"

int CRTScreen::hPos = 0;
int CRTScreen::vPos = 0;
BYTE CRTScreen::Pixels[DataSize];
bool CRTScreen::frameFinished;

void CRTScreen::Init()
{
    hPos = 0;
    vPos = 0;
    frameFinished = false;
}

void CRTScreen::Clock()
{
    hPos ++;
    if (GateArray::hsyncTrigger || hPos > 3090) // 16068 Hz free running
    {
        hPos = 0;
        vPos ++;
        GateArray::hsyncTrigger = false;
    }
    if ((GateArray::vsyncTrigger && vPos > 300) || vPos > 325) // 48Hz free running
    {
        vPos = 0;
        frameFinished = true;
        GateArray::vsyncTrigger = false;
    }
    unsigned int base = vPos * Stride + hPos * 3;
    if (base < DataSize)
    {
        Pixels[base] = GateArray::Color[0];
        Pixels[base + 1] = GateArray::Color[1];
        Pixels[base + 2] = GateArray::Color[2];
    }
}


#include "CRTScreen.h"
#include "GateArray.h"
#include "CRTC.h"

int CRTScreen::hPos = 0;
int CRTScreen::vPos = 0;
int CRTScreen::hSyncPos = 0;
BYTE CRTScreen::Pixels[DataSize];
bool CRTScreen::frameFinished;
unsigned int CRTScreen::pixelIndex;
int CRTScreen::hOffset = 0;

void CRTScreen::Init()
{
    hPos = 0;
    vPos = 0;
    hOffset = 0;
    frameFinished = false;
}

void CRTScreen::Clock()
{
    hPos ++;
    if (GateArray::hsyncTrigger)
    {
        hPos = 0;
        vPos ++;
        switch (CRTC::HSW)
        {
        case 3:
            hOffset = 72;
            break;
        case 4:
            hOffset = 48;
            break;
        case 5:
            hOffset = 24;
            break;
        default:
            hOffset = 0;
            break;
        }
        GateArray::hsyncTrigger = false;
        if (GateArray::vsyncTrigger)
        {
            vPos = 0;
            frameFinished = true;
            GateArray::vsyncTrigger = false;
            pixelIndex = 0;
        }
    }
    pixelIndex = vPos * Stride + hPos * 3;
    if (hOffset + pixelIndex < DataSize)
    {
        Pixels[hOffset + pixelIndex++] = GateArray::Color[0];
        Pixels[hOffset + pixelIndex++] = GateArray::Color[1];
        Pixels[hOffset + pixelIndex++] = GateArray::Color[2];
    }
}

void CRTScreen::OneMhzClock()
{
}

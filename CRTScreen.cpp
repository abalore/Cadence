#include "CRTScreen.h"
#include "GateArray.h"
#include "CRTC.h"

int CRTScreen::hPos = 0;
int CRTScreen::vPos = 0;
int CRTScreen::hSyncPos = 0;
BYTE CRTScreen::Pixels[DataSize];
bool CRTScreen::frameFinished;
unsigned int CRTScreen::pixelIndex;

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
        if (GateArray::vsyncTrigger || vPos > 325) // 48Hz free running
        {
            vPos = 0;
            frameFinished = true;
            GateArray::vsyncTrigger = false;
            switch (CRTC::HSW)
            {
            case 3:
                pixelIndex = 72;
                break;
            case 4:
                pixelIndex = 48;
                break;
            case 5:
                pixelIndex = 24;
                break;
            default:
                pixelIndex = 0;
                break;
            }
        }
    }
    //pixelIndex = vPos * Stride + hPos * 3;
    if (pixelIndex < DataSize)
    {
        Pixels[pixelIndex++] = GateArray::Color[0];
        Pixels[pixelIndex++] = GateArray::Color[1];
        Pixels[pixelIndex++] = GateArray::Color[2];
    }
}

void CRTScreen::OneMhzClock()
{
}

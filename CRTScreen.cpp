#include "CRTScreen.h"
#include "GateArray.h"
#include "CRTC.h"

int CRTScreen::hPos = 0;
int CRTScreen::vPos = 0;
BYTE CRTScreen::Pixels[DataSize];
bool CRTScreen::frameFinished;
int CRTScreen::hOffset = 0;
unsigned int CRTScreen::writeOffset = 0;

void CRTScreen::Init()
{
    hPos = 0;
    vPos = 0;
    hOffset = 0;
    frameFinished = false;
    writeOffset = 0;
}

void CRTScreen::Clock()
{
    hPos++;
    if (GateArray::hsyncTrigger)
    {
        hPos = 0;
        vPos++;
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
        }
        writeOffset = (unsigned int)(hOffset + vPos * Stride);
    }
    if (writeOffset + 3 <= (unsigned int)DataSize)
    {
        const BYTE *c = GateArray::Color;
        Pixels[writeOffset]     = c[0];
        Pixels[writeOffset + 1] = c[1];
        Pixels[writeOffset + 2] = c[2];
    }
    writeOffset += 3;
}

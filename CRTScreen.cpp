#include "CRTScreen.h"
#include "CPC.h"

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
    if (CPC::gateArray.hsyncTrigger)
    {
        hPos = 0;
        vPos++;
        switch (CPC::crtc.HSW)
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
        CPC::gateArray.hsyncTrigger = false;
        if (CPC::gateArray.vsyncTrigger)
        {
            vPos = 0;
            frameFinished = true;
            CPC::gateArray.vsyncTrigger = false;
        }
        writeOffset = (unsigned int)(hOffset + vPos * Stride);
    }
    if (writeOffset + 3 <= (unsigned int)DataSize)
    {
        const BYTE *c = CPC::gateArray.Color;
        Pixels[writeOffset]     = c[0];
        Pixels[writeOffset + 1] = c[1];
        Pixels[writeOffset + 2] = c[2];
    }
    writeOffset += 3;
}

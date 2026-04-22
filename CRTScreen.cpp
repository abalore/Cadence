#include "CRTScreen.h"
#include "CPC.h"

void CRTScreen::Init()
{
    hPos = 0;
    vPos = 0;
    hOffset = 0;
    frameFinished = false;
    hPhaseFP = 0;
}

void CRTScreen::Clock()
{
    hPhaseFP += FPOne;
    if (hPhaseFP >= NominalLineLenFP)
        hPhaseFP -= NominalLineLenFP;

    if (CPC::gateArray.hsyncTrigger)
    {
        int err = hPhaseFP;
        if (err >= NominalLineLenFP / 2)
            err -= NominalLineLenFP;
        hPhaseFP -= err / PLLGainDenom;
        if (hPhaseFP < 0)
            hPhaseFP += NominalLineLenFP;
        else if (hPhaseFP >= NominalLineLenFP)
            hPhaseFP -= NominalLineLenFP;

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
    }

    hPos = hPhaseFP >> FPShift;
    unsigned int wOff = (unsigned int)(vPos * Stride + hOffset + hPos * BytesPerPixel);
    if (wOff + 3 <= (unsigned int)DataSize)
    {
        const BYTE *c = CPC::gateArray.Color;
        Pixels[wOff]     = c[0];
        Pixels[wOff + 1] = c[1];
        Pixels[wOff + 2] = c[2];
    }
}

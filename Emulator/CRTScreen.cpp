#include "Headers/CRTScreen.h"
#include "Headers/CRTC.h"
#include "Headers/GateArray.h"

int CRTScreen::hPos = 0;
int CRTScreen::vPos = 0;
BYTE CRTScreen::Pixels[PixelWidth * PixelHeight * BytesPerPixel * 2];

void CRTScreen::Clock()
{
    if (hPos < PixelWidth && vPos < PixelHeight)
    {
        int base = vPos * Stride * 2 + hPos * BytesPerPixel;
        for (int i = 0; i < BytesPerPixel; i++)
            Pixels[base + i] = GateArray::Color[i];
    }

    if (CRTC::HSYNC)
    {
        if (hPos != 0)
            vPos++;
        hPos = 0;
    }
    else
        hPos++;

    if (CRTC::VSYNC) vPos = 0;

}


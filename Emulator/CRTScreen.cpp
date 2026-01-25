#include "Headers/CRTScreen.h"
#include "Headers/CRTC.h"
#include "Headers/GateArray.h"

int CRTScreen::hPos = 0;
int CRTScreen::vPos = 0;
BYTE CRTScreen::Pixels[PixelWidth * PixelHeight * BytesPerPixel * 2];
bool CRTScreen::lastHSYNC = false;
bool CRTScreen::lastVSYNC = false;

void CRTScreen::Clock()
{
    if (hPos < PixelWidth && vPos < PixelHeight)
    {
        int base = vPos * Stride * 2 + hPos * BytesPerPixel;
        for (int i = 0; i < BytesPerPixel; i++)
            Pixels[base + i] = GateArray::Color[i];
    }

    if (CRTC::HSYNC && !lastHSYNC)
    {
        if (hPos != 0)
            vPos++;
        hPos = 0;
    }
    else
        hPos++;

    if (CRTC::VSYNC && !lastVSYNC) vPos = 0;

    lastHSYNC = CRTC::HSYNC;
    lastVSYNC = CRTC::VSYNC;

}


#include "Headers/CRTScreen.h"
#include "Headers/GateArray.h"

int CRTScreen::hPos = 0;
int CRTScreen::vPos = 0;
BYTE CRTScreen::Pixels[PixelWidth * PixelHeight * BytesPerPixel * 2];
bool CRTScreen::lastHSYNC = false;
bool CRTScreen::lastVSYNC = false;

void CRTScreen::Init()
{
    hPos = 0;
    vPos = 0;
    lastHSYNC = false;
    lastVSYNC = false;
}

void CRTScreen::Clock()
{
    hPos++;
    if (GateArray::GA_HSYNC && !lastHSYNC)
    {
        hPos = 0;
        vPos++;
    }
    if (GateArray::GA_VSYNC && !lastVSYNC)
        vPos = 0;
    if (hPos < PixelWidth && vPos < PixelHeight)
    {
        int base = vPos * Stride * 2 + hPos * BytesPerPixel;
        for (int i = 0; i < BytesPerPixel; i++)
            Pixels[base + i] = GateArray::Color[i];
    }
    lastHSYNC = GateArray::GA_HSYNC;
    lastVSYNC = GateArray::GA_VSYNC;
}


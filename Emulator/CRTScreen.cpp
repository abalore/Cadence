#include "Headers/CRTScreen.h"
#include "Headers/GateArray.h"

int CRTScreen::hPos = 0;
int CRTScreen::vPos = 0;
BYTE CRTScreen::Pixels[PixelWidth * PixelHeight * BytesPerPixel];
bool CRTScreen::lastHSYNC = false;
bool CRTScreen::lastVSYNC = false;
CRTStage CRTScreen::stage = CRTStage::Running;

void CRTScreen::Init()
{
    hPos = 0;
    vPos = 0;
    lastHSYNC = false;
    lastVSYNC = false;
}

void CRTScreen::Clock()
{
    hPos += 3;
    if (!GateArray::GA_HSYNC && lastHSYNC)
    {
        hPos = 0;
        vPos += Stride;
    }
    if (GateArray::GA_VSYNC && !lastVSYNC)
    {
        vPos = 0;
        stage = CRTStage::WaitingEmulation;
    }
    int base = vPos + hPos;
    if (base < DataSize)
    {
        Pixels[base] = GateArray::Color[0];
        Pixels[base + 1] = GateArray::Color[1];
        Pixels[base + 2] = GateArray::Color[2];
    }

    lastHSYNC = GateArray::GA_HSYNC;
    lastVSYNC = GateArray::GA_VSYNC;
}


#include "Headers/CRTScreen.h"
#include "Headers/GateArray.h"

int CRTScreen::hPos = 0;
int CRTScreen::vPos = 0;
BYTE CRTScreen::Pixels[PixelWidth * PixelHeight * BytesPerPixel];
CRTStage CRTScreen::stage = CRTStage::Running;

void CRTScreen::Init()
{
    hPos = 0;
    vPos = 0;
}

void CRTScreen::Clock()
{
    hPos += 3;
    if (GateArray::hsyncTrigger/* || hPos == 1024*/)
    {
        hPos = 0;
        vPos += Stride;
        GateArray::hsyncTrigger = false;
    }
    if ((GateArray::vsyncTrigger/* || vPos == 312*/) && vPos)
    {
        vPos = 0;
        stage = CRTStage::WaitingEmulation;
        GateArray::vsyncTrigger = false;
    }
    int base = vPos + hPos;
    if (base < DataSize)
    {
        Pixels[base] = GateArray::Color[0];
        Pixels[base + 1] = GateArray::Color[1];
        Pixels[base + 2] = GateArray::Color[2];
    }
}


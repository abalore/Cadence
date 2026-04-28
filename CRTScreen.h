#ifndef CRTSCREEN_H
#define CRTSCREEN_H

#include "defs.h"

class CRTScreen
{
public:
    void Init();
    void Clock();
    static constexpr int PixelWidth = 1024;
    static constexpr int PixelHeight = 312;
    static constexpr int BytesPerPixel = 3;
    static constexpr int Stride = PixelWidth * BytesPerPixel;
    static constexpr int DataSize = PixelWidth * PixelHeight * BytesPerPixel;
    BYTE Pixels[DataSize];
    bool frameFinished;
    int hPos, vPos;
private:
    int hOffset;
    int hPhaseFP;
    int clocksSinceTrigger;
    static constexpr int FPShift = 8;
    static constexpr int FPOne = 1 << FPShift;
    static constexpr int NominalLineLen = PixelWidth;
    static constexpr int NominalLineLenFP = NominalLineLen << FPShift;
    static constexpr int PLLGainDenom = 16;
    // Flywheel: if no HSYNC arrives within ~1.1 nominal lines, generate one ourselves.
    static constexpr int FlywheelTimeout = NominalLineLen + NominalLineLen / 8;
};

#endif // CRTSCREEN_H

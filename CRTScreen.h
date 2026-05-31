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
    int hFlywheelCounter = 0;
    static constexpr int FPShift = 8;
    static constexpr int FPOne = 1 << FPShift;
    static constexpr int NominalLineLen = PixelWidth;
    static constexpr int NominalLineLenFP = NominalLineLen << FPShift;
    static constexpr int PLLGainDenom = 16;
    // Horizontal flywheel lock window: only a sync arriving past SyncWindowMin
    // clocks counts as a real line; earlier pulses (e.g. a second HSYNC injected
    // mid-scanline by moving R2/R3) are ignored. If no sync arrives by FreeRunMax
    // the flywheel free-runs and retraces anyway.
    static constexpr int SyncWindowMin = NominalLineLen * 7 / 8; // 896
    static constexpr int FreeRunMax = NominalLineLen * 5 / 4;    // 1280
};

#endif // CRTSCREEN_H

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
    unsigned int writeOffset;
};

#endif // CRTSCREEN_H

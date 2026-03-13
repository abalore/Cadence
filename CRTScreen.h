#ifndef CRTSCREEN_H
#define CRTSCREEN_H

#include "defs.h"

class CRTScreen
{
public:
    static void Init();
    static void Clock();
    static void OneMhzClock();
    static const int PixelWidth = 1024;
    static const int PixelHeight = 312;
    static const int BytesPerPixel = 3;
    static const int Stride = PixelWidth * BytesPerPixel;
    static const int DataSize = PixelWidth * PixelHeight * BytesPerPixel;
    static BYTE Pixels[DataSize];
    static bool frameFinished;
    static int hPos, vPos;
private:
    static unsigned int pixelIndex;
    static int hSyncPos;
};

#endif // CRTSCREEN_H

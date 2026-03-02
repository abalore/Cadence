#ifndef CRTC_H
#define CRTC_H

#include "defs.h"

class CRTC
{
public:
    static void Reset();
    static void Clock();
    static void RD();
    static void WR();
    static BYTE Registers[18];
    static BYTE Index;
    static BYTE RA;
    static word MA;
    static bool HSYNC, VSYNC, BORDER;
    static word HCC, VCC;
    static BYTE HSC, VSC;
    static BYTE R12, R13;
    static BYTE verticalAdjust;
private:
    static void Update();
};

#endif // CRTC_H

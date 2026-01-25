#ifndef CRTC_H
#define CRTC_H

#include "defs.h"

class CRTC
{
public:
    static void Init();
    static void CRTClock();
    static void IOClock();
    static BYTE Registers[18];
    static BYTE Index;
    static BYTE RA;
    static word MA;
    static bool HSYNC, VSYNC, BORDER, VSYNCSTART, HSyncFallingEdge;
    static word HCC, VCC;
    static BYTE HSC, VSC;
};

#endif // CRTC_H

#ifndef CRTC_H
#define CRTC_H

#include "defs.h"

class CRTC
{
public:
    static void Init();
    static void Clock();
    static void RD();
    static void WR();
    static BYTE Registers[18];
    static BYTE Index;
    static BYTE RA;
    static word MA;
    static bool HSYNC, VSYNC, BORDER, WILLVSYNC;
    static word HCC, VCC;
    static BYTE HSC, VSC;
    static BYTE R12, R13;
private:
    static void CheckHSync();
};

#endif // CRTC_H

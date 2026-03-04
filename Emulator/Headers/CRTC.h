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
    static BYTE R[18];
    static BYTE Index;
    static BYTE RA;
    static bool HSYNC, VSYNC, BORDER;

    static BYTE HCC;
    static word VLC;
    static BYTE VCC;
    static BYTE VSC;
    static BYTE HSC;
    static word MA;

    static BYTE VTAC;
    static BYTE verticalTotal;
    static BYTE crtcType;

    static BYTE R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15, R16, R17;

private:
    static void RunHorizontalChar();
    static void RunAdj();
    static void RunLine();
    static void RunVerticalChar();
    static void ResetFrame();
    static bool HDISP;
    static bool VDISP;
    static word baseMA;
    static bool adjustMode;

};

#endif // CRTC_H

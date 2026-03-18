#ifndef CRTC_H
#define CRTC_H

#include "defs.h"
#include "Counter.h"
#include "DFlipFlop.h"
#include "RSFlipFlop.h"

class CRTC
{
public:
    static void Reset();
    static void Clock();
    static void RD();
    static void WR();
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

    static BYTE HT, HD, HSP, HSW, VSW, VT, VTA, VD, VSP, IS, MRA;

    static bool HDISP;
    static bool VDISP;
    static word baseMA;
    static bool adjustMode;
    static word DSA;
    static BYTE newVT;
    static bool EndLine, EndChar, EndScreen;


private:
    static void RunHorizontalChar();
    static void RunAdj();
    static void RunLine();
    static void RunVerticalChar();
    static void ResetFrame();
    static void RunCombinational();

};

#endif // CRTC_H

#ifndef CRTC_H
#define CRTC_H

#include "defs.h"

class CRTC
{
public:
    static void Reset();
    static void Clock();
    static BYTE RD(BYTE address);
    static void WR(BYTE address, BYTE value);
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
    static void EndOfLine();

};

#endif // CRTC_H

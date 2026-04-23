#ifndef CRTC_H
#define CRTC_H

#include "defs.h"

class CRTC
{
public:
    void Reset();
    void Clock();
    BYTE RD(BYTE address);
    void WR(BYTE address, BYTE value);
    BYTE Index;
    BYTE RA;
    bool HSYNC, VSYNC, BORDER;

    BYTE HCC;
    word VLC;
    BYTE VCC;
    BYTE VSC;
    BYTE HSC;
    word MA;

    BYTE VTAC;
    BYTE verticalTotal;
    BYTE crtcType = 0;

    BYTE HT, HD, HSP, HSW, VSW, VT, VTA, VD, VSP, IS, MRA;
    BYTE CH = 0, CL = 0;


    bool HDISP;
    bool VDISP;
    word baseMA;
    bool adjustMode;
    word DSA;
    BYTE newVT;
    bool EndLine, EndChar, EndScreen;

private:
    bool willAdjust = false;
    bool vccWrapped = false; // set when VCC wraps 0x7F→0x00 during a frame; cleared on frame reset
    void RunHorizontalChar();
    void RunAdj();
    void EndOfLine();

};

#endif // CRTC_H

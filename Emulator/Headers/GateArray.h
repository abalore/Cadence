#ifndef GATEARRAY_H
#define GATEARRAY_H

#include "defs.h"

class GateArray
{
public:
    static void Init();
    static void Clock();
    static bool ROMEN();
    static bool RAMRD();
    static bool MWE();
    static BYTE Color[3];
    static BYTE INK[16];
    static BYTE BORDER;
    static BYTE currentPen;
    static const BYTE cH = 255;
    static const BYTE cM = 128;
    static const BYTE cL = 0;
    static BYTE RMR;
    static BYTE hsyncCounter;
    static BYTE vsyncIntDelay;
    static BYTE mode;
    constexpr static const BYTE Palette[3 * 32] =
        {
            cM, cM, cM, // White
            cM, cM, cM, // White
            cL, cH, cM, // Sea green
            cH, cH, cM, // Pastel Yellow
            cL, cL, cM, // Blue
            cH, cL, cM, // Purple
            cL, cM, cM, // Cyan
            cH, cM, cM, // Pink
            cH, cL, cM, // Purple
            cH, cH, cM, // Pastel yellow
            cH, cH, cL, // Bright yellow
            cH, cH, cH, // Bright white
            cH, cL, cL, // Bright red
            cH, cL, cH, // Bright magenta
            cH, cM, cL, // Orange
            cH, cM, cH, // Pastel magenta
            cL, cL, cM, // Blue
            cL, cH, cM, // Sea green
            cL, cH, cL, // Bright green
            cL, cH, cH, // Bright cyan
            cL, cL, cL, // Black
            cL, cL, cH, // Bright blue
            cL, cM, cL, // Green
            cL, cM, cH, // Sky blue
            cM, cL, cM, // Magenta
            cM, cH, cM, // Pastel green
            cM, cH, cL, // Lime
            cM, cH, cH, // Pastel cyan
            cM, cL, cL, // Red
            cM, cL, cH, // Mauve
            cM, cM, cL, // Yellow
            cL, cL, cH, // Pastel blue
    };

private:
    static void SetPixel();
    static void ReadByte();
    static void IO_Clock();
    // static void PrintDebugLine();
    static BYTE MMR;
    static bool borderSelected;
    static word videoAddress;
    static BYTE currentByte;
    static BYTE pixelIndex;
    static BYTE videoPen;
    static BYTE clockDividerCounter;
    static bool CCLK;
    static bool lastHSYNC;
};

#endif // GATEARRAY_H

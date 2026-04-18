#ifndef GATEARRAY_H
#define GATEARRAY_H

#include "defs.h"

class GateArray
{
public:
    static void Reset();
    static void ProcessSync();
    static BYTE GetPenForPixel(BYTE m, BYTE b, BYTE i);
    static const BYTE *GetPaletteEntry(BYTE entry);
    static void WR(BYTE value);
    static void AckInt();
    static void SetPixel();
    static void ReadByte(bool lo);
    static void LoadVideoAddress();
    static const BYTE *Color;
    static BYTE INK[16];
    static BYTE BORDER;
    static BYTE currentPen;
    static const BYTE cL = 5;
    static const BYTE cMR = 127;
    static const BYTE cMG = 127;
    static const BYTE cMB = 127;
    static const BYTE cHR = 254;
    static const BYTE cHG = 254;
    static const BYTE cHB = 254;
    static BYTE R52;
    static BYTE mode;
    static BYTE pi;
    static BYTE decodedPen[4][8][256];
    static bool hsyncTrigger;
    static bool vsyncTrigger;
    static bool LoROMActive;
    static bool HiROMActive;
    static bool Monochrome;
    static bool CCLK;
    static bool VideoAccess;
    static BYTE porch;
    static word videoAddress;
    static BYTE ready;

    constexpr static const BYTE AbsoluteBlack[3] = {0, 100, 0};
    constexpr static const BYTE NormalBlack[3] = {cL, cL, cL};
    constexpr static const BYTE Palette[3 * 32] =
        {
            cMR, cMG, cMB, // White
            cMR, cMG, cMB, // White
            cL, cHG, cMB, // Sea green
            cHR, cHG, cMB, // Pastel Yellow
            cL, cL, cMB, // Blue
            cHR, cL, cMB, // Purple
            cL, cMG, cMB, // Cyan
            cHR, cMG, cMB, // Pink
            cHR, cL, cMB, // Purple
            cHR, cHG, cMB, // Pastel yellow
            cHR, cHG, cL, // Bright yellow
            cHR, cHG, cHB, // Bright white
            cHR, cL, cL, // Bright red
            cHR, cL, cHB, // Bright magenta
            cHR, cMG, cL, // Orange
            cHR, cMG, cHB, // Pastel magenta
            cL, cL, cMB, // Blue
            cL, cHG, cMB, // Sea green
            cL, cHG, cL, // Bright green
            cL, cHG, cHB, // Bright cyan
            cL, cL, cL, // Black
            cL, cL, cHB, // Bright blue
            cL, cMG, cL, // Green
            cL, cMG, cHB, // Sky blue
            cMR, cL, cMB, // Magenta
            cMR, cHG, cMB, // Pastel green
            cMR, cHG, cL, // Lime
            cMR, cHG, cHB, // Pastel cyan
            cMR, cL, cL, // Red
            cMR, cL, cHB, // Mauve
            cMR, cMG, cL, // Yellow
            cMR, cMG, cHB, // Pastel blue
    };

    static constexpr BYTE GreenPalette[3 * 32]
        {
            0,159,0, 0,159,0, 0,206,0, 0,248,0, 0,25,0, 0,103,0, 0,132,0, 0,183,0,
            0,103,0, 0,248,0, 0,241,0, 0,255,0, 0,92,0, 0,113,0, 0,175,0, 0,191,0,
            0,25,0, 0,206,0, 0,198,0, 0,213,0, 0,0,0, 0,42,0, 0,123,0, 0,141,0,
            0,69,0, 0,227,0, 0,220,0, 0,234,0, 0,56,0, 0,81,0, 0,150,0, 0,167,0
        };

private:
    static BYTE RMR;
    static BYTE MMR;
    static bool borderSelected;
    static word currentWord;
    static BYTE pixelIndex;
    static BYTE currentInk;
    static bool lastHSYNC;
    static bool lastVSYNC;
    static bool lastHDISP;
    static BYTE hsyncDelay;
    static BYTE vsyncDelay;
    static BYTE intTimeout;
    static BYTE latchLo;
    static BYTE latchHi;
    static bool dispEnFF1;
    static bool dispEnFF2;
    static BYTE nextMode;
};

#endif // GATEARRAY_H


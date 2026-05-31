#ifndef GATEARRAY_H
#define GATEARRAY_H

#include "defs.h"

struct GateArrayDebugState
{
    BYTE currentPen, BORDER, mode, R52;
    word videoAddress;
    BYTE INK[16];
    bool LoROMActive, HiROMActive;
};

class GateArray
{
public:
    void Reset();
    void ProcessSync();
    BYTE GetPenForPixel(BYTE m, BYTE b, BYTE i);
    const BYTE *GetPaletteEntry(BYTE entry);
    const BYTE *GetBorderPaletteEntry();
    void WR(BYTE value);
    void AckInt();
    void SetPixel();
    void ReadByte(bool lo);
    void LoadVideoAddress();
    void SetMonochrome(bool m);
    inline BYTE GetMode() const { return mode; }
    GateArrayDebugState GetDebugState() const;

    // Bus/memory-map signals consumed by CRTScreen and CPC::UpdateMemoryMap
    const BYTE *Color;
    bool hsyncTrigger;
    bool vsyncTrigger;
    bool LoROMActive;
    bool HiROMActive;

    static constexpr BYTE cL = 5;
    static constexpr BYTE cMR = 127;
    static constexpr BYTE cMG = 127;
    static constexpr BYTE cMB = 127;
    static constexpr BYTE cHR = 254;
    static constexpr BYTE cHG = 254;
    static constexpr BYTE cHB = 254;

    static constexpr BYTE AbsoluteBlack[3] = {0, 0, 0};
    static constexpr BYTE NormalBlack[3] = {cL, cL, cL};
    static constexpr BYTE Palette[3 * 32] =
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
    BYTE INK[16];
    BYTE BORDER;
    BYTE currentPen;
    BYTE R52;
    BYTE mode;
    BYTE pi;
    BYTE decodedPen[4][8][256];
    bool Monochrome = false;
    bool CCLK;
    bool VideoAccess;
    BYTE porch;
    word videoAddress;
    BYTE ready;
    BYTE RMR;
    BYTE MMR;
    bool borderSelected;
    word currentWord;
    BYTE pixelIndex;
    const BYTE *blankColor;
    const BYTE *currentPalette;
    bool lastHSYNC;
    bool lastVSYNC;
    BYTE hsyncDelay;
    BYTE vsyncDelay;
    BYTE intTimeout;
    BYTE latchLo;
    BYTE latchHi;
    bool dispEnFF1;
    bool dispEnFF2;
    BYTE nextMode;
};

#endif // GATEARRAY_H

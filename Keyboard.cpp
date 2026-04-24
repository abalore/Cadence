#include "Keyboard.h"
#include <cstring>

#ifdef __APPLE__
// macOS virtual key codes (kVK_*) → CPC matrix values
BYTE Keyboard::translation[128] =
    { 58, 47, 57, 56, 45, 46, 78, 77, 67, 76,     //  0- 9  A S D F H G Z X C V
     255, 66, 38, 37, 27, 26, 35, 36,  8, 18,     // 10-19  § B Q W E R Y T 1 2
      17,  7,  6, 16,  3, 14, 15, 13,  5,  4,     // 20-29  3 4 6 5 = 9 7 - 8 0
      12, 24, 25, 23, 34, 33, 22, 44, 55, 43,     // 30-39  ] O U [ I P RET L J '
      54, 53, 32, 74, 63, 65, 64, 73, 48, 75,     // 40-49  K ; \ , / N M . TAB SPACE
      62, 79,255, 28,255,255, 52, 68, 11, 72,     // 50-59  ` BS — ESC — — LSHIFT CAPS LOPT LCTRL
      52,255, 72,255,255, 70,255,255,255,255,     // 60-69  RSHIFT — RCTRL — — KP. — — — —
     255,255,255,255,255,255, 60,255,255,255,     // 70-79  — — — — — — KPENTER — — —
     255,255, 71, 51, 61, 50, 42, 41, 40, 21,     // 80-89  — — KP0 KP1 KP2 KP3 KP4 KP5 KP6 KP7
      31, 30,255,255,255,255,255,255,255,255,     // 90-99  KP8 KP9 — — — — F5 F6 F7 F3
     255,255,255,255,255,255,255,255,255,255,     //100-109 F8 F9 — F11 — F13 F16 F14 — F10
     255,255,255,255,255,255,255, 79,255,255,     //110-119 — F12 — — — — — FWDDEL — —
     255,255,255,  1, 10, 20,  0,255              //120-127 — — F1 LEFT RIGHT DOWN UP —
    };
#else
// X11 scan codes (evdev, offset by 9) → CPC matrix values
BYTE Keyboard::translation[128] =
    { 28, 8, 18, 17, 7, 16, 6, 15, 5, 14, 4, 13, 3, 79,   // 0 - 13  ESC 1234567890 - CLR DEL
        48, 38, 37, 27, 26, 36, 35, 25, 34, 24, 33, 23, 12,   // 14 - 26  TAB QWERTYUIOP @ [
        22, 72, // 27 - 28  RETURN CONTROL
        58, 47, 57, 56, 46, 45, 55, 54, 44, 53, 43, // 29 - 39  ASDFGHJKL : ;
        62, 52, 32, // 40 - 42  \ SHIFT ]
        78, 77, 67, 76, 66, 65, 64, 74, 73, 63, // 43 - 52  ZXCVBNM , . /
        62, 255, 11, 75, 68, // 53 - 57  SHIFT ??? COPY SPACE CAPSLOCK
        255,255,255,255,255,10,255,255,255,255, // 58 - 67   F keys
        255, 255, // 68 - 69  NUMLOCK SCROLLLOCK
        21, 31, 30,  59, 42, 41, 40, 49, 51, 61, 50, 71, 70, // 70 - 82 Keypad: 789 KP-/JoyFire2 456 KP+/JoyFire1 1230 .
        255, 255, 255, 255, 255, 255, 255, 49, 255, 255, 255, 255, // 83 - 94
        60, // 95
        255, 255, 255, 255, 255, 9, 0, 39, 1, 10, 19, 20, 255, 29, // 96 - 109
        2, 255, 255, 255, 255, 255, 255, 255, 255, 255 // 110  CLR ?? ?? ?? ?? ??
    };
#endif

void Keyboard::Reset()
{
    memset(matrix, 0xFF, sizeof(matrix));
}

BYTE Keyboard::Read()
{
    matrixLock.lock();
    BYTE value = matrix[Row];
    matrixLock.unlock();
    return value;
}

void Keyboard::SetRow(BYTE row)
{
    if (row < 10)
        Row = row;
}

void Keyboard::KeyEvent(int key, bool release)
{
#ifdef __APPLE__
    if (key < 0 || key > 127) return;
    BYTE cpcKey = translation[key];
#else
    if (key < 9 || key > 136) return;
    BYTE cpcKey = translation[key - 9];
#endif
    if (cpcKey >= 80) return;
    int row = cpcKey % 10;
    int bit = 1 <<(cpcKey / 10);
    matrixLock.lock();
    if (release)
        matrix[row] |= bit;
    else
        matrix[row] &= bit ^ 0xFF;
    matrixLock.unlock();

}

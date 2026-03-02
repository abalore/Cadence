#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "defs.h"
#include <mutex>

using namespace std;

class Keyboard
{
public:
    static void Reset();
    static BYTE Read();
    static void SetRow(BYTE row);
    static void KeyEvent(int key, bool release);
    static BYTE Row;
private:
    static BYTE matrix[10];
    constexpr static BYTE translation[128] =
        { 28, 8, 18, 17, 7, 16, 6, 15, 5, 14, 4, 13, 3, 79,   // 0 - 13  ESC 1234567890 - CLR DEL
            48, 38, 37, 27, 26, 36, 35, 25, 34, 24, 33, 23, 12,   // 14 - 26  TAB QWERTYUIOP @ [
            22, 72, // 27 - 28  RETURN CONTROL
            58, 47, 57, 56, 46, 45, 55, 54, 44, 53, 43, // 29 - 39  ASDFGHJKL : ;
            62, 52, 32, // 40 - 42  \ SHIFT ]
            78, 77, 67, 76, 66, 65, 64, 74, 73, 63, // 43 - 52  ZXCVBNM , . /
            52, 255, 11, 75, 68, // 53 - 57  SHIFT ??? COPY SPACE CAPSLOCK
            255,255,255,255,255,10,255,255,255,255, // 58 - 67   F keys
            255, 255, // 68 - 69  NUMLOCK SCROLLLOCK
            21, 31, 30, 255, 42, 41, 40, 49, 51, 61, 50, 71, 70, // 70 - 82 Keypad: 789 End->Joy1Down 456 JoyFire1 1230 .
            255, 255, 255, 255, 255, 255, 255, 49, 255, 255, 255, 255, // 83 - 94
            60, // 95
            255, 255, 255, 255, 255, 9, 0, 39, 1, 10, 19, 20, 255, 29, // 96 - 109
            2, 255, 255, 255, 255, 255, 255, 255, 255, 255 // 110  CLR ?? ?? ?? ?? ??
    };
    static mutex matrixLock;
};

#endif // KEYBOARD_H

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
    static BYTE translation[128];
private:
    static BYTE matrix[10];
    static mutex matrixLock;
};

#endif // KEYBOARD_H

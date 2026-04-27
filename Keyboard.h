#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "defs.h"
#include <atomic>
#include <mutex>

class Keyboard
{
public:
    void Reset();
    BYTE Read();
    void SetRow(BYTE row);
    void KeyEvent(int key, bool release);
    BYTE Row;
    static BYTE translation[128];
    static std::atomic<bool> joystickEmulation;
private:
    BYTE matrix[10];
    std::mutex matrixLock;
};

#endif // KEYBOARD_H

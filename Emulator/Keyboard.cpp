#include "Headers/Keyboard.h"

BYTE Keyboard::Row;
BYTE Keyboard::matrix[10];
BYTE Keyboard::translation[128] =
    { 28, 8, 18, 17, 7, 16, 6, 15, 5, 14, 4, 13, 3, 79,
    48, 38, 37, 27, 26, 36, 35, 25, 34, 24, 33, 23, 12,
    22, 72,
    58, 47, 57, 56, 46, 45, 55, 54, 44, 53, 43,
    62, 52, 32,
    78, 77, 67, 76, 66, 65, 64, 74, 73, 63,
    52, 255, 11, 75, 68,
    255,255,255,255,255,255,255,255,255,255, // F keys
    255, 255, // Num-lock Scroll-lock
    21, 31, 30, 255, 42, 41, 40, 255, 51, 61, 50, 71, 70,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    60,
    255, 255, 255, 255, 255, 255, 0, 255, 1, 10, 255, 20, 255, 255,
    2
};
mutex Keyboard::matrixLock;

bool pressed = false;

void Keyboard::Init()
{
    for (int i = 0; i < 10; i++)
        matrix[i] = 0xFF;
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
    BYTE cpcKey = translation[key - 9];
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

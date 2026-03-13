#include "Keyboard.h"

BYTE Keyboard::Row;
BYTE Keyboard::matrix[10];
mutex Keyboard::matrixLock;

bool pressed = false;

void Keyboard::Reset()
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

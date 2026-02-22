#ifndef EMULATOR_H
#define EMULATOR_H

enum CPCType
{
    CPC464,
    CPC664,
    CPC6128
};

class Emulator
{
public:
    static void Init();
    static void Finalize();
    static void Clock();
    static void Reset();
    static void ReadROM(char *filename, int number);
    static CPCType cpcType;
};

#endif // EMULATOR_H

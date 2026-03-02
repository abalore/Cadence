#ifndef EMULATOR_H
#define EMULATOR_H

class Emulator
{
public:
    static void Init();
    static void Finalize();
    static void Clock();
    static void Reset();
};

#endif // EMULATOR_H

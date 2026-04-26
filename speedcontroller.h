#ifndef SPEEDCONTROLLER_H
#define SPEEDCONTROLLER_H

#include <atomic>

enum SpeedMode
{
    Keep,
    Slowdown,
    Speedup
};

class SpeedController
{
public:
    static void Run();
    static std::atomic<long> lastElapsed;
    static std::atomic<bool> unlocked;
    static bool end;
    static bool overrun;
private:
    static long lastT;
    static long t;
    static long elapsed;
    static int targetTime;
    static SpeedMode speedMode;
};

#endif // SPEEDCONTROLLER_H

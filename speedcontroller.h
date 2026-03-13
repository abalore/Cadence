#ifndef SPEEDCONTROLLER_H
#define SPEEDCONTROLLER_H

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
    static volatile long lastElapsed;
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

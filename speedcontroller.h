#ifndef SPEEDCONTROLLER_H
#define SPEEDCONTROLLER_H

class SpeedController
{
public:
    static void Run();
    static volatile long lastElapsed;
    static bool end;
private:
    static long lastT;
    static long t;
    static long elapsed;
};

#endif // SPEEDCONTROLLER_H

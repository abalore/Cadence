#include "speedcontroller.h"
#include "SoundThread.h"
#include <chrono>
#include <QThread>

using namespace std::chrono;

std::atomic<long> SpeedController::lastElapsed{0};

long SpeedController::lastT = 0;
long SpeedController::t = 0;
long SpeedController::elapsed = 0;
bool SpeedController::overrun = false;
int SpeedController::targetTime = 20000;
SpeedMode SpeedController::speedMode = SpeedMode::Keep;

void SpeedController::Run()
{
    long frames = SoundThread::frames;
    switch(speedMode)
    {
    case SpeedMode::Keep:
        if (frames > 4000)
        {
            speedMode = SpeedMode::Slowdown;
            targetTime = 20100;
        }
        else if (frames < 3000)
        {
            speedMode = SpeedMode::Speedup;
            targetTime = 19900;
        }
        break;
    case SpeedMode::Slowdown:
    case SpeedMode::Speedup:
        if (frames > 3400 && frames < 3600)
        {
            targetTime = 20000;
            speedMode = SpeedMode::Keep;
        }
        break;
    }

    duration now = high_resolution_clock::now().time_since_epoch();
    t = duration_cast<microseconds>(now).count();
    elapsed = t - lastT;
    lastElapsed = elapsed;

    overrun = false;
    if (elapsed > targetTime)
        overrun = true;
    else
    {
        while (elapsed < targetTime)
        {
            QThread::usleep(0);
            duration now = high_resolution_clock::now().time_since_epoch();
            t = duration_cast<microseconds>(now).count();
            elapsed = t - lastT;
        }
    }
    lastT = t;
}

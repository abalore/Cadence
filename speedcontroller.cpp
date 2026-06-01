#include "speedcontroller.h"
#include "SoundThread.h"
#include <QElapsedTimer>
#include <thread>

// Monotonic, high-resolution time source. QElapsedTimer uses QueryPerformance-
// Counter on Windows (sub-µs) and clock_gettime(MONOTONIC) on Linux, avoiding
// the coarse (~15.6 ms) default Windows timer that std::chrono / QThread::usleep
// can be hostage to.
static QElapsedTimer s_timer;

std::atomic<long> SpeedController::lastElapsed{0};
std::atomic<bool> SpeedController::unlocked{false};

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

    if (!s_timer.isValid())
    {
        s_timer.start();
        lastT = 0;
    }

    t = s_timer.nsecsElapsed() / 1000;
    elapsed = t - lastT;
    lastElapsed = elapsed;

    overrun = false;
    if (unlocked || elapsed > targetTime)
        overrun = true;
    else
    {
        while (elapsed < targetTime)
        {
            std::this_thread::yield();
            t = s_timer.nsecsElapsed() / 1000;
            elapsed = t - lastT;
        }
    }
    lastT = t;
}

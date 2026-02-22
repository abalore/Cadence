#include "speedcontroller.h"
#include <chrono>
#include <QThread>

using namespace std::chrono;

volatile long SpeedController::lastElapsed;

long SpeedController::lastT = 0;
long SpeedController::t = 0;
long SpeedController::elapsed = 0;

void SpeedController::Run()
{
    duration now = high_resolution_clock::now().time_since_epoch();
    t = duration_cast<microseconds>(now).count();
    elapsed = t - lastT;
    lastElapsed = elapsed;
    if (elapsed > 19968)
    {
        QThread::usleep(0);
    }
    else
    {
        while (elapsed < 19968)
        {
            QThread::usleep(0);
            duration now = high_resolution_clock::now().time_since_epoch();
            t = duration_cast<microseconds>(now).count();
            elapsed = t - lastT;
        }
    }
    lastT = t;
}

#include "EmulatorThread.h"
#include "CPC.h"
#include "Z80.h"
#include "CRTScreen.h"
#include "Disassembler.h"
#include "SoundThread.h"
#include "speedcontroller.h"

std::atomic<ushort> EmulatorThread::stopPoint{0x1CB3}; //0x4921;
std::atomic<bool> EmulatorThread::running{true};
std::atomic<RunMode> EmulatorThread::runMode{RunMode::StopPoint};
std::atomic<bool> EmulatorThread::end{false};
QMutex EmulatorThread::frameMutex;

EmulatorThread::EmulatorThread(QObject *parent) : QThread(parent)
{
    CPC::Init();
    Disassembler::Init();
}

EmulatorThread::~EmulatorThread()
{
    CPC::Finalize();
}

void EmulatorThread::Pause()
{
    if (running)
        runMode = RunMode::StepByStep;
}

void EmulatorThread::Run()
{
    if (!running)
    {
        runMode = RunMode::Run;
        running = true;
    }
}

void EmulatorThread::RunStep()
{
    if (!running)
    {
        runMode = RunMode::StepByStep;
        running = true;
    }
}

void EmulatorThread::RunTo(ushort address)
{
    if (!running)
    {
        stopPoint = address;
        runMode = RunMode::StopPoint;
        running = true;
    }
}

void EmulatorThread::Stop ()
{
    CPC::z80.stopPoint = false;
    running = false;
    paused = true;
    emit OnPause();
}

void EmulatorThread::run()
{
    paused = false;
    running = true;
    CPC::Reset();
    if (runMode == RunMode::Run && CPC::Breakpoint[CPC::z80.GetPC()])
        Stop();
    while (!end)
    {
        if (!paused)
        {
            frameMutex.lock();
            while (!CPC::screen.frameFinished && running)
            {
                CPC::Clock();
                if (CPC::z80.stopPoint && CPC::tick % 4 == 0)
                    switch(runMode)
                    {
                    case RunMode::StepByStep:
                        Stop();
                        break;
                    case RunMode::StopPoint:
                        if (CPC::z80.GetPC() == stopPoint)
                            Stop();
                        break;
                    case RunMode::Run:
                        if (CPC::Breakpoint[CPC::z80.GetPC()])
                            Stop();
                        break;
                    }
            }
            CPC::screen.frameFinished = false;
            frameMutex.unlock();
            SoundThread::waitCondition.wakeOne();
            emit OnFinishedFrame();
            SpeedController::Run();
        }
        else
        {
            if (running)
            {
                paused = false;
                emit OnResume();
            }
            QThread::msleep(5);
        }
    }
}

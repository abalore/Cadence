#include "EmulatorThread.h"
#include "Emulator.h"
#include "CPC.h"
#include "Z80.h"
#include "CRTScreen.h"
#include "SoundThread.h"
#include "speedcontroller.h"

volatile ushort EmulatorThread::stopPoint = 0x0938;
volatile bool EmulatorThread::running = true;
volatile RunMode EmulatorThread::runMode = RunMode::Run;
volatile bool EmulatorThread::end = false;

EmulatorThread::EmulatorThread(QObject *parent) : QThread(parent)
{
    Emulator::Init();
}

EmulatorThread::~EmulatorThread()
{
    Emulator::Finalize();
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
    Z80::stopPoint = false;
    running = false;
    emit OnPause();
}

void EmulatorThread::run()
{
    Emulator::Reset();
    //Emulator::Breakpoint[0x89F] = true;
    while (!end)
    {
        if (running)
        {
            while (!CRTScreen::frameFinished && running)
            {
                Emulator::Clock();
                if (Z80::stopPoint && CPC::tick % 4 == 0)
                    switch(runMode)
                    {
                    case RunMode::StepByStep:
                        Stop();
                        break;
                    case RunMode::StopPoint:
                        if (Z80::PC == stopPoint)
                            Stop();
                        break;
                    case RunMode::Run:
                        if (Emulator::Breakpoint[Z80::PC])
                            Stop();
                        break;
                    }
            }
            CRTScreen::frameFinished = false;
            SoundThread::waitCondition.wakeOne();
            emit OnFinishedFrame();
            SpeedController::Run();
        }
        else QThread::msleep(5);
    }
}

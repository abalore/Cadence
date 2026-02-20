#include "EmulatorThread.h"
#include "Emulator/Headers/Emulator.h"
#include "Emulator/Headers/Z80.h"
#include "Emulator/Headers/CRTScreen.h"
#include "SoundThread.h"
#include "speedcontroller.h"

volatile ushort EmulatorThread::stopPoint = 0xC6E7;
volatile bool EmulatorThread::running = true;
volatile RunMode EmulatorThread::runMode = RunMode::Run;
volatile bool EmulatorThread::end = false;

EmulatorThread::EmulatorThread(QObject *parent) : QThread(parent)
{
    Emulator::Init();
}

EmulatorThread::~EmulatorThread()
{

}

void EmulatorThread::Pause()
{
    if (running)
    {
        runMode = RunMode::StepByStep;
    }
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
    while (!end)
    {
        if (running)
        {
            while (!CRTScreen::frameFinished)
            {
                Emulator::Clock();
                switch(runMode)
                {
                case RunMode::StepByStep:
                    if (Z80::stopPoint)
                        Stop();
                    break;
                case RunMode::StopPoint:
                    if (Z80::stopPoint && Z80::PC == stopPoint)
                        Stop();
                    break;
                case RunMode::Run:
                    break;
                }
            }
            CRTScreen::frameFinished = false;
            SoundThread::waitCondition.wakeOne();
            emit OnFinishedFrame();
            SpeedController::Run();
        }
    }
}

void EmulatorThread::Reset()
{
    running = false;
    Emulator::Reset();
    running = true;
}

#include "EmulatorWorkerThread.h"
#include "Emulator/Headers/Emulator.h"
#include "Emulator/Headers/Z80.h"
#include "Emulator/Headers/CRTScreen.h"
#include "SoundThread.h"
#include "speedcontroller.h"

volatile ushort EmulatorWorkerThread::stopPoint = 0xC6E7;
volatile bool EmulatorWorkerThread::running = true;
volatile RunMode EmulatorWorkerThread::runMode = RunMode::Run;
volatile bool EmulatorWorkerThread::end = false;

EmulatorWorkerThread::EmulatorWorkerThread(QObject *parent) : QThread(parent)
{
    Emulator::Init();
}

EmulatorWorkerThread::~EmulatorWorkerThread()
{

}

void EmulatorWorkerThread::Pause()
{
    if (running)
    {
        runMode = RunMode::StepByStep;
    }
}

void EmulatorWorkerThread::Run()
{
    if (!running)
    {
        runMode = RunMode::Run;
        running = true;
    }
}

void EmulatorWorkerThread::RunStep()
{
    if (!running)
    {
        runMode = RunMode::StepByStep;
        running = true;
    }
}

void EmulatorWorkerThread::RunTo(ushort address)
{
    if (!running)
    {
        stopPoint = address;
        runMode = RunMode::StopPoint;
        running = true;
    }
}

void EmulatorWorkerThread::Stop ()
{
    Z80::stopPoint = false;
    running = false;
    emit OnPause();
}

void EmulatorWorkerThread::run()
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

void EmulatorWorkerThread::Reset()
{
    running = false;
    Emulator::Reset();
    running = true;
}

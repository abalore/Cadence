#include "EmulatorWorkerThread.h"
#include "Emulator/Headers/Emulator.h"
#include "Emulator/Headers/Z80.h"
#include "Emulator/Headers/CPC.h"
#include "Emulator/Headers/CRTC.h"
#include "Emulator/Headers/GateArray.h"
#include "Emulator/Headers/CRTScreen.h"
#include "Emulator/Headers/PPI.h"
#include "Emulator/Headers/Tape.h"

volatile ushort EmulatorWorkerThread::stopPoint = 0x0000;
volatile bool EmulatorWorkerThread::running = true;
RunMode EmulatorWorkerThread::runMode = RunMode::Run;
volatile bool EmulatorWorkerThread::canDebug = true;
volatile bool EmulatorWorkerThread::end = false;
unsigned char EmulatorWorkerThread::nextInstructionLength;
mutex EmulatorWorkerThread::debugLock;
volatile long EmulatorWorkerThread::elapsed;

string EmulatorWorkerThread::debugStringZ80 = "Z80";
string EmulatorWorkerThread::debugStringStack = "Stack";
string EmulatorWorkerThread::debugStringCRTC = "CRTC";
string EmulatorWorkerThread::debugStringGateArray = "GateArray";

EmulatorWorkerThread::EmulatorWorkerThread(QObject *parent) : QThread(parent) { }

string EmulatorWorkerThread::GetZ80RegsDebugLine()
{
    string d;
    char buff[200];
    sprintf(buff, "AF %04X\nBC %04X\nDE %04X\nHL %04X\nPC %04X\nSP %04X\nIX %04X\nIY %04X\nSZ-H-PNC\n%08b\n",
            Z80::AF.Get(), Z80::BC.Get(), Z80::DE.Get(), Z80::HL.Get(),
            Z80::PC, Z80::SP.Get(), Z80::IX.Get(), Z80::IY.Get(), Z80::F);
    d.append(buff);
    sprintf(buff, "IM:%1d\nInts:%1d", Z80::InterruptMode, Z80::InterruptEnable);
    d.append(buff);
    //    d.append("AF'  BC'  DE'  HL'  R    I    Ints\n");
    //    sprintf(buff, "%04X %04X %04X %04X %02X   %02X   %01X\n",
    //            Z80::AF_.Get(), Z80::BC_.Get(), Z80::DE_.Get(), Z80::HL_.Get(), Z80::R, Z80::I, Z80::InterruptEnable);
    //    d.append(buff);
    return d;
}

string EmulatorWorkerThread::GetZ80StackDebugLine()
{
    string d;
    char buff[100];
    word sp = Z80::SP.Get();
    for (int i = 0; i < 8; i++)
    {
        BYTE L = CPC::InternalRAM->MEM[sp];
        BYTE H = CPC::InternalRAM->MEM[sp + 1];
        sprintf(buff, "%04X : %04X\n", sp, L + H * 256);
        d.append(buff);
        sp += 2;
    }
    return d;
}

string EmulatorWorkerThread::GetCRTCDebugLine()
{
    string crtc;
    char buff[100];
    for (int i = 0; i < 18; i++)
    {
        sprintf(buff, "%2d ", i);
        crtc += (string)buff;
    }
    crtc += "\n";
    for (int i = 0; i < 18; i++)
    {
        sprintf(buff, "%02X ", CRTC::Registers[i]);
        crtc += (string)buff;
    }
    crtc += "\n";
    sprintf(buff, "HCC: %02d  VCC: %02d  HSYNC: %1d  VSYNC: %1d", CRTC::HCC, CRTC::VCC, CRTC::HSYNC, CRTC::VSYNC);
    crtc += buff;
    return crtc;
}

string EmulatorWorkerThread::GetGateArrayDebugLine()
{
    string d;
    char buff[100];
    sprintf(buff, "Pen: %d   Border: %d\nInks: ", GateArray::currentPen, GateArray::BORDER);
    d.append(buff);
    for (int i = 0; i < 16; i++)
    {
        sprintf(buff, "%02X ", GateArray::INK[i] + 0x40);
        d.append(buff);
    }
    sprintf(buff, "\nRMR: %08b  R52: %d  PPI Control: %08b", GateArray::RMR, GateArray::R52, PPI::controlWord);
    d += buff;
    return d;
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
    debugLock.lock();
    debugStringZ80 = GetZ80RegsDebugLine();
    debugStringStack = GetZ80StackDebugLine();
    debugStringCRTC = GetCRTCDebugLine();
    debugStringGateArray = GetGateArrayDebugLine();
    debugLock.unlock();
    running = false;
    OnPause();
}

void EmulatorWorkerThread::run()
{
    Emulator::Init();
    bool lastMotorState = Tape::motorState;

    while(!end)
    {
        if (running)
        {
            switch(CRTScreen::stage)
            {
            case CRTStage::Running:
                Emulator::Clock();
                /*
                if (Tape::motorState)
                    OnTapeMotorOn();
                else
                    OnTapeMotorOff();
*/
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
                break;
            case CRTStage::WaitingEmulation:
                OnFinishedFrame();
                CRTScreen::stage = CRTStage::WaitingAudio;
                break;
            case CRTStage::WaitingAudio:
                break;
            }
        }
        else
            sleep(0);
    }
}

void EmulatorWorkerThread::Reset()
{
    running = false;
    Emulator::Reset();
    running = true;
}

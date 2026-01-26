#include "EmulatorWorkerThread.h"
#include "Emulator/Headers/Emulator.h"
#include "Emulator/Headers/Z80.h"
#include "Emulator/Headers/CPC.h"
#include "Emulator/Headers/CRTC.h"
#include "Emulator/Headers/GateArray.h"
#include "Emulator/Headers/Disassembler.h"

volatile ushort EmulatorWorkerThread::stopPoint = 0xC006; //0x35F2; //0xC006; //0x0C6B; //0xBDD9; //
volatile bool EmulatorWorkerThread::running = true;
RunMode EmulatorWorkerThread::runMode = RunMode::StopPoint;
volatile bool EmulatorWorkerThread::canDebug = true;
volatile int EmulatorWorkerThread::iteration;
volatile int EmulatorWorkerThread::measures;
volatile int EmulatorWorkerThread::total;
volatile bool EmulatorWorkerThread::end = false;
unsigned char EmulatorWorkerThread::nextInstructionLength;
mutex EmulatorWorkerThread::debugLock;

string EmulatorWorkerThread::debugStringZ80 = "Z80";
string EmulatorWorkerThread::debugStringStack = "Stack";
string EmulatorWorkerThread::debugStringDisassembler = "Disassembler";
string EmulatorWorkerThread::debugStringCRTC = "CRTC";
string EmulatorWorkerThread::debugStringGateArray = "GateArray";
string EmulatorWorkerThread::debugStringMem = "Memory";

EmulatorWorkerThread::EmulatorWorkerThread(QObject *parent) : QThread(parent) { }

string EmulatorWorkerThread::GetZ80RegsDebugLine()
{
    string d;
    char buff[100];
    d.append("AF   BC   DE   HL   PC   SP   IX   IY   SZ-H-PNC\n");
    sprintf(buff, "%04X %04X %04X %04X %04X %04X %04X %04X %08b\n",
            Z80::AF.Get(), Z80::BC.Get(), Z80::DE.Get(), Z80::HL.Get(),
            Z80::PC, Z80::SP.Get(), Z80::IX.Get(), Z80::IY.Get(), Z80::F);
    d.append(buff);
    d.append("AF'  BC'  DE'  HL'  R    I    Ints\n");
    sprintf(buff, "%04X %04X %04X %04X %02X   %02X   %01X\n",
            Z80::AF_.Get(), Z80::BC_.Get(), Z80::DE_.Get(), Z80::HL_.Get(), Z80::R, Z80::I, Z80::InterruptEnable);
    d.append(buff);
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
        sprintf(buff, " %04X : %04X\n", sp, L + H * 256);
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
        sprintf(buff, "%3d ", i);
        crtc += (string)buff;
    }
    crtc += "\n";
    for (int i = 0; i < 18; i++)
    {
        sprintf(buff, "%3d ", CRTC::Registers[i]);
        crtc += (string)buff;
    }
    crtc += "\n";
    sprintf(buff, "HCC: %3d  VCC: %3d  HSYNC: %1d  VSYNC: %1d", CRTC::HCC, CRTC::VCC, CRTC::HSYNC, CRTC::VSYNC);
    crtc += buff;
    return crtc;
}

string EmulatorWorkerThread::GetDisassembly()
{
    string address;
    string bytes;
    string instruction;
    string disassembly;
    BYTE length;
    Disassembler::SetPoint(Z80::PC);
    for (int i = 0; i < 10; i++)
    {
        Disassembler::GetNextInstruction(length, &address, &bytes, &instruction);
        bytes.insert(bytes.size(), 13 - bytes.size(), ' ');
        disassembly += address + "  " + bytes + instruction + "\n";
        if (i == 0)
            nextInstructionLength = length;
    }
    return disassembly;
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
    sprintf(buff, "\nRMR: %08b  R52: %d", GateArray::RMR, GateArray::hsyncCounter);
    d += buff;
    return d;
}

string EmulatorWorkerThread::GetMemDebugLine()
{
    string d;
    char buff[10];
    for (int i = 0xAE20; i < 0xAE60; i ++)
    {
        if ((i % 16) == 0)
        {
            sprintf(buff, "%04X", i);
            if (i > 0) d += "\n";
            d += buff;
            d += "  ";
        }
        sprintf(buff, "%02X", CPC::InternalRAM->MEM[i]);
        d += buff;
        d += " ";
    }
    return d;
}

void EmulatorWorkerThread::Run()
{
    runMode = RunMode::Run;
    running = true;
}

void EmulatorWorkerThread::Pause()
{
    runMode = RunMode::StepByStep;
}

void EmulatorWorkerThread::StepIn()
{
    runMode = RunMode::StepByStep;
    running = true;
}

void EmulatorWorkerThread::StepOut()
{
    word address = Z80::SP.Get();
    BYTE L = CPC::InternalRAM->MEM[address];
    BYTE H = CPC::InternalRAM->MEM[address + 1];
    stopPoint = L + H * 256;
    runMode = RunMode::StepByStep;
    running = true;
}

void EmulatorWorkerThread::StepOver()
{
    stopPoint = Z80::PC + nextInstructionLength;
    runMode = RunMode::StepByStep;
    running = true;
}

void EmulatorWorkerThread::StartDebugging ()
{
    debugLock.lock();
    debugStringZ80 = GetZ80RegsDebugLine();
    debugStringStack = GetZ80StackDebugLine();
    debugStringDisassembler = GetDisassembly();
    debugStringCRTC = GetCRTCDebugLine();
    debugStringGateArray = GetGateArrayDebugLine();
    debugStringMem = GetMemDebugLine();
    debugLock.unlock();
    running = false;
    OnPause();

}

void EmulatorWorkerThread::run()
{
    Emulator::Init();
    while(!end)
    {
        if (running)
        {
            Emulator::Clock();
            iteration++;
            switch(runMode)
            {
            case RunMode::StepByStep:
                if (Z80::stopPoint)
                    StartDebugging();
                break;
            case RunMode::StopPoint:
                if (Z80::stopPoint && Z80::PC == stopPoint)
                    StartDebugging();
                break;
            case RunMode::Run:
                break;
            }

        }
        else
            sleep(0);
    }
}

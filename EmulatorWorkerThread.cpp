#include "EmulatorWorkerThread.h"
#include "Emulator/Headers/Emulator.h"
#include "Emulator/Headers/Z80.h"
#include "Emulator/Headers/CPC.h"
#include "Emulator/Headers/CRTC.h"
#include "Emulator/Headers/GateArray.h"
#include "Emulator/Headers/Disassembler.h"

volatile bool EmulatorWorkerThread::running = true;
volatile ushort EmulatorWorkerThread::stopPoint = 0x0000; //0xC006; //0x0C6B; //0xBDD9; //
volatile bool EmulatorWorkerThread::stepByStep = false;
volatile bool EmulatorWorkerThread::canDebug = false;
volatile int EmulatorWorkerThread::iteration;
volatile int EmulatorWorkerThread::measures;
volatile int EmulatorWorkerThread::total;

string EmulatorWorkerThread::debugStringZ80 = "Z80";
string EmulatorWorkerThread::debugStringStack = "Stack";
string EmulatorWorkerThread::debugStringDisassembler = "Disassembler";
string EmulatorWorkerThread::debugStringCRTC = "CRTC";
string EmulatorWorkerThread::debugStringGateArray = "GateArray";

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
        byte L = CPC::InternalRAM->MEM[sp];
        byte H = CPC::InternalRAM->MEM[sp + 1];
        sprintf(buff, " %04X : %04X\n", sp, L + H * 256);
        d.append(buff);
        sp += 2;
    }
    return d;
}

string EmulatorWorkerThread::GetCRTCDebugLine()
{
    string crtc;
    char buff[10];
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
    return crtc;
}

string EmulatorWorkerThread::GetDisassembly()
{
    string address;
    string bytes;
    string instruction;
    string disassembly;
    Disassembler::SetPoint(Z80::PC);
    for (int i = 0; i < 10; i++)
    {
        Disassembler::GetNextInstruction(&address, &bytes, &instruction);
        bytes.insert(bytes.size(), 16 - bytes.size(), ' ');
        disassembly += address + bytes + instruction + "\n";
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

void EmulatorWorkerThread::Run()
{
    stepByStep = false;
    canDebug = false;
    running = true;
}

void EmulatorWorkerThread::Pause()
{
    stepByStep = false;
    canDebug = true;
    running = false;
}

void EmulatorWorkerThread::StepIn()
{
    stepByStep = true;
    canDebug = false;
    running = true;
}

void EmulatorWorkerThread::StepOut()
{
    stepByStep = false;
    word address = Z80::SP.Get();
    byte L = CPC::InternalRAM->MEM[address];
    byte H = CPC::InternalRAM->MEM[address + 1];
    stopPoint = L + H * 256;
    canDebug = false;
    running = true;
}

void EmulatorWorkerThread::StepOver()
{
    stepByStep = false;
    stopPoint = Z80::PC + 2;
    canDebug = false;
    running = true;
}

void EmulatorWorkerThread::run()
{
    Emulator::Init();
    while(true)
    {
        if (running)
        {
            if (canDebug
                && !Z80::M1
                && Z80::tCycle == 1
                && Z80::idMode == IDMode::BASIC
                && ((Z80::PC == stopPoint) || stepByStep))
            {
                // Z80::debugStringLock.lock();
                debugStringZ80 = GetZ80RegsDebugLine();
                debugStringStack = GetZ80StackDebugLine();
                debugStringDisassembler = GetDisassembly();
                debugStringCRTC = GetCRTCDebugLine();
                debugStringGateArray = GetGateArrayDebugLine();
                // Z80::debugStringLock.unlock();
                running = false;
                stepByStep = true;
                OnPause();
            }
            else
            {
                Emulator::Clock();
                iteration++;
            }

        } else sleep(0);
    }
}

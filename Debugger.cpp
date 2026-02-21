#include "Debugger.h"
#include "ui_Debugger.h"
#include "EmulatorThread.h"
#include "Emulator/Headers/Disassembler.h"
#include "Emulator/Headers/Z80.h"
#include "Emulator/Headers/PPI.h"
#include "Emulator/Headers/CPC.h"
#include "Emulator/Headers/CRTC.h"
#include "Emulator/Headers/GateArray.h"
#include <QCloseEvent>
#include <QStringListModel>
#include <QModelIndex>
#include <QScrollBar>
#include <QShortcut>

Debugger::Debugger(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Debugger)
{
    ui->setupUi(this);
    connect(ui->btnStepOut, &QPushButton::clicked, this, &Debugger::onStepOutClicked);
    connect(ui->btnRun, &QPushButton::clicked, this, &Debugger::onRunClicked);
    connect(ui->btnStepOver, &QPushButton::clicked, this, &Debugger::onStepOverClicked);
    connect(ui->btnStepIn, &QPushButton::clicked, this, &Debugger::onStepInClicked);
    connect(ui->btnRunTo, &QPushButton::clicked, this, &Debugger::onRunToClicked);
    modelDisassembly = new QStringListModel();
    ui->listDisassembly->setModel(modelDisassembly);
    modelMemory = new QStringListModel();
    ui->listMemory->setModel(modelMemory);
    connect(new QShortcut(Qt::Key_F8, this), &QShortcut::activated, this, &Debugger::onStepOutClicked);
    connect(new QShortcut(Qt::Key_F9, this), &QShortcut::activated, this, &Debugger::onRunClicked);
    connect(new QShortcut(Qt::Key_F10, this), &QShortcut::activated, this, &Debugger::onStepOverClicked);
    connect(new QShortcut(Qt::Key_F11, this), &QShortcut::activated, this, &Debugger::onStepInClicked);
    connect(new QShortcut(Qt::Key_F12, this), &QShortcut::activated, this, &Debugger::onRunToClicked);
}

Debugger::~Debugger()
{
    delete modelMemory;
    delete modelDisassembly;
    delete ui;
}

void Debugger::closeEvent(QCloseEvent *event)
{
    EmulatorThread::Run();
    setHidden(true);
    event->ignore();
}

void Debugger::Update()
{
    int pcPosition = 0;
    int instrCount = 0;
    listDisassembly.clear();
    char buff[200];
    bool pcFound = false;
    Disassembler::SetPoint(0x0000);
    while (Disassembler::addr < 0x10000)
    {
        BYTE opCode;
        BYTE instrLength;
        string label, address, bytes, instruction;
        ushort position = Disassembler::addr;
        Disassembler::GetNextInstruction(instrLength, opCode, &label, &address, &bytes, &instruction);
        sprintf(buff, "%16s  %4s  %12s  %s", label.data(), address.data(), bytes.data(), instruction.data());
        listDisassembly.append(buff);
        if (!pcFound && position >= Z80::PC)
        {
            pcPosition = instrCount;
            pcFound = true;
            nextInstructionLength = instrLength;
            nextInstructionOpCode = opCode;
        }
        instrCount++;
    }
    modelDisassembly -> setStringList(listDisassembly);
    modelDisassemblyIndex = modelDisassembly->index(pcPosition);
    ui->listDisassembly->scrollTo(modelDisassemblyIndex, QAbstractItemView::PositionAtCenter);
    ui->listDisassembly->setCurrentIndex(modelDisassemblyIndex);

    int memoryIndex = ui->listMemory->currentIndex().row();
    listMemory.clear();
    for (int i = 0x0000; i <= 0xFFF0; i += 16)
    {
        sprintf(buff, "%04X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
                i, CPC::BaseRAM[i], CPC::BaseRAM[i + 1],
                CPC::BaseRAM[i + 2], CPC::BaseRAM[i + 3],
                CPC::BaseRAM[i + 4], CPC::BaseRAM[i + 5],
                CPC::BaseRAM[i + 6], CPC::BaseRAM[i + 7],
                CPC::BaseRAM[i + 8], CPC::BaseRAM[i + 9],
                CPC::BaseRAM[i + 10], CPC::BaseRAM[i + 11],
                CPC::BaseRAM[i + 12], CPC::BaseRAM[i + 13],
                CPC::BaseRAM[i + 14], CPC::BaseRAM[i + 15]);
        listMemory.append(buff);
    }

    modelMemory -> setStringList(listMemory);
    modelMemoryIndex = modelMemory->index(memoryIndex);
    ui->listMemory->scrollTo(modelMemoryIndex, QAbstractItemView::PositionAtCenter);
    ui->listMemory->setCurrentIndex(modelMemoryIndex);

    string debugStringZ80;
    string debugStringStack;
    string debugStringCRTC;
    string debugStringGateArray;

    debugStringZ80 = GetZ80RegsDebugLine();
    debugStringStack = GetZ80StackDebugLine();
    debugStringCRTC = GetCRTCDebugLine();
    debugStringGateArray = GetGateArrayDebugLine();

    ui->lblZ80->setText(debugStringZ80.data());
    ui->lblStack->setText(debugStringStack.data());
    ui->lblCRTC->setText(debugStringCRTC.data());
    ui->lblGateArray->setText(debugStringGateArray.data());

    setEnabled(true);
}

void Debugger::onRunClicked()
{
    hide();
    EmulatorThread::Run();
}

void Debugger::onStepInClicked()
{
    EmulatorThread::RunStep();
}

void Debugger::onStepOverClicked()
{
    setEnabled(false);
    if (nextInstructionOpCode == 0x18
        || nextInstructionOpCode == 0xC3
        || nextInstructionOpCode == 0xC9
        || nextInstructionOpCode == 0xE9)
        EmulatorThread::RunStep();
    else
        EmulatorThread::RunTo(Z80::PC + nextInstructionLength);
}

void Debugger::onStepOutClicked()
{
    setEnabled(false);
    word address = Z80::SP.Get();
    BYTE L = CPC::BaseRAM[address];
    BYTE H = CPC::BaseRAM[address + 1];
    EmulatorThread::RunTo(L + H * 256);
}

void Debugger::onRunToClicked()
{
    setEnabled(false);
    int index = ui->listDisassembly->currentIndex().row();
    QString string = listDisassembly.at(index).mid(18, 4);
    EmulatorThread::RunTo(string.toInt(nullptr, 16));
}

string Debugger::GetZ80RegsDebugLine()
{
    string d;
    char buff[200];
    sprintf(buff, "AF %04X\nBC %04X\nDE %04X\nHL %04X\nPC %04X\nSP %04X\nIX %04X\nIY %04X\nSZ-H-PNC\n%1b%1b%1b%1b%1b%1b%1b%1b\n",
            Z80::AF.Get(), Z80::BC.Get(), Z80::DE.Get(), Z80::HL.Get(),
            Z80::PC, Z80::SP.Get(), Z80::IX.Get(), Z80::IY.Get(),
            Z80::fS, Z80::fZ, Z80::f5, Z80::fH, Z80::f3, Z80::fP, Z80::fN, Z80::fC);
    d.append(buff);
    sprintf(buff, "IM:%1d\nInts:%1d", Z80::InterruptMode, Z80::InterruptEnable);
    d.append(buff);
    return d;
}

string Debugger::GetZ80StackDebugLine()
{
    string d;
    char buff[100];
    word sp = Z80::SP.Get();
    for (int i = 0; i < 8; i++)
    {
        BYTE L = CPC::BaseRAM[sp];
        BYTE H = CPC::BaseRAM[sp + 1];
        sprintf(buff, "%04X : %04X\n", sp, L + H * 256);
        d.append(buff);
        sp += 2;
    }
    return d;
}

string Debugger::GetCRTCDebugLine()
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

string Debugger::GetGateArrayDebugLine()
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
    sprintf(buff, "\nLoR: %1b  HiR: %1b  R52: %d  PPI Control: %08b", GateArray::LoROMActive, GateArray::HiROMActive, GateArray::R52, PPI::controlWord);
    d += buff;
    return d;
}

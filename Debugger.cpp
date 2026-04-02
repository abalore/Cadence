#include "Debugger.h"
#include "ui_Debugger.h"
#include "Emulator.h"
#include "EmulatorThread.h"
#include "Disassembler.h"
#include "Z80.h"
#include "PPI.h"
#include "CPC.h"
#include "CRTC.h"
#include "CRTScreen.h"
#include "GateArray.h"
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
    connect(ui->btnResetNops, &QPushButton::clicked, this, &Debugger::onResetNopsClicked);

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
        sprintf(buff, "%s %14s  %4s  %12s  %s", Emulator::Breakpoint[position] ? "* " : "  ", label.data(), address.data(), bytes.data(), instruction.data());
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
        int bank = i >> 14;
        int address = i & 0x3FFF;
        BYTE *mem = CPC::RAM[bank];
        sprintf(buff, "%04X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
                i, mem[address], mem[address + 1],
                mem[address + 2], mem[address + 3],
                mem[address + 4], mem[address + 5],
                mem[address + 6], mem[address + 7],
                mem[address + 8], mem[address + 9],
                mem[address + 10], mem[address + 11],
                mem[address + 12], mem[address + 13],
                mem[address + 14], mem[address + 15]);
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
   // hide();
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
    word address = Z80::SP;
    BYTE L = CPC::GetByteAt(address);
    BYTE H = CPC::GetByteAt(address + 1);
    EmulatorThread::RunTo(L + H * 256);
}

void Debugger::onResetNopsClicked()
{
    Z80::nops = 0;
    Update();
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
    sprintf(buff, "AF %04X\nBC %04X\nDE %04X\nHL %04X\nPC %04X\nSP %04X\nIX %04X\nIY %04X\nSZ-H-PNC\n%1b%1b%1b%1b%1b%1b%1b%1b\nIRQ: %1d\nIFF1: %1d\nIFF2: %1d\n",
            Z80::AF.Get(), Z80::BC.Get(), Z80::DE.Get(), Z80::HL.Get(),
            Z80::PC, Z80::SP, Z80::IX.Get(), Z80::IY.Get(),
            Z80::fS, Z80::fZ, Z80::f5, Z80::fH, Z80::f3, Z80::fP, Z80::fN, Z80::fC, Z80::InterruptRequest, Z80::IFF1, Z80::IFF2);
    d.append(buff);
    sprintf(buff, "R:%02X I:%02X\nIM:%1d\nInts:%1d\nNOPS:%d", Z80::R, Z80::I, Z80::im, Z80::IFF1, Z80::nops);
    d.append(buff);
    return d;
}

string Debugger::GetZ80StackDebugLine()
{
    string d;
    char buff[100];
    word sp = Z80::SP;
    for (int i = 0; i < 8; i++)
    {
        BYTE L = CPC::GetByteAt(sp);
        BYTE H = CPC::GetByteAt(sp + 1);
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
    sprintf(buff, "HT  %3d  HCC %3d\n", CRTC::HT, CRTC::HCC); crtc += buff;
    sprintf(buff, "HD  %3d  HDISP %1d\n", CRTC::HD, CRTC::HDISP); crtc += buff;
    sprintf(buff, "HSP %3d  HSYNC %1d\n", CRTC::HSP, CRTC::HSYNC); crtc += buff;
    sprintf(buff, "HSW %3d  HSC %3d\n", CRTC::HSW, CRTC::HSC); crtc += (string) buff;
    sprintf(buff, "VSW %3d  VSC %3d\n", CRTC::VSW, CRTC::VSC); crtc += (string) buff;
    sprintf(buff, "VT  %3d  VCC %3d\n", CRTC::VT, CRTC::VCC); crtc += buff;
    sprintf(buff, "VD  %3d  VDISP %1d\n", CRTC::VD, CRTC::VDISP); crtc += buff;
    sprintf(buff, "VSP %3d  VSYNC %1d\n", CRTC::VSP, CRTC::VSYNC); crtc += buff;
    sprintf(buff, "MRA %3d  RA  %3d\n", CRTC::MRA, CRTC::RA); crtc += buff;
    sprintf(buff, "VTA %3d  VTAC %2d\n", CRTC::VTA, CRTC::VTAC); crtc += buff;
    sprintf(buff, "SA %04X  MA %04X\n", CRTC::DSA, CRTC::MA); crtc += (string) buff;
    sprintf(buff, "sX %4d sY %4d\n", CRTScreen::hPos, CRTScreen::vPos); crtc += (string) buff;
    return crtc;
}

string Debugger::GetGateArrayDebugLine()
{
    string d;
    char buff[100];
    sprintf(buff, "Pen: %d   Border: %d  Mode: %d  Video address: %04X\nInks: ", GateArray::currentPen, GateArray::BORDER, GateArray::mode, GateArray::videoAddress);
    d.append(buff);
    for (int i = 0; i < 16; i++)
    {
        sprintf(buff, "%02X ", GateArray::INK[i] + 0x40);
        d.append(buff);
    }
    sprintf(buff, "\nLoR: %1b  HiR: %1b  R52: %d  PPI Control: %08b  Window: %d", GateArray::LoROMActive, GateArray::HiROMActive, GateArray::R52, PPI::controlWord, (CPC::tick % 16) >> 2);
    d += buff;
    return d;
}

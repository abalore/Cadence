#include "Debugger.h"
#include "ui_Debugger.h"
#include "EmulatorWorkerThread.h"
#include "Emulator/Headers/Disassembler.h"
#include "Emulator/Headers/Z80.h"
#include "Emulator/Headers/CPC.h"
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
    EmulatorWorkerThread::Run();
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
                i, CPC::InternalRAM->MEM[i], CPC::InternalRAM->MEM[i + 1],
                CPC::InternalRAM->MEM[i + 2], CPC::InternalRAM->MEM[i + 3],
                CPC::InternalRAM->MEM[i + 4], CPC::InternalRAM->MEM[i + 5],
                CPC::InternalRAM->MEM[i + 6], CPC::InternalRAM->MEM[i + 7],
                CPC::InternalRAM->MEM[i + 8], CPC::InternalRAM->MEM[i + 9],
                CPC::InternalRAM->MEM[i + 10], CPC::InternalRAM->MEM[i + 11],
                CPC::InternalRAM->MEM[i + 12], CPC::InternalRAM->MEM[i + 13],
                CPC::InternalRAM->MEM[i + 14], CPC::InternalRAM->MEM[i + 15]);
        listMemory.append(buff);
    }

    modelMemory -> setStringList(listMemory);
    modelMemoryIndex = modelMemory->index(memoryIndex);
    ui->listMemory->scrollTo(modelMemoryIndex, QAbstractItemView::PositionAtCenter);
    ui->listMemory->setCurrentIndex(modelMemoryIndex);

    EmulatorWorkerThread::debugLock.lock();
    ui->lblZ80->setText(EmulatorWorkerThread::debugStringZ80.data());
    ui->lblStack->setText(EmulatorWorkerThread::debugStringStack.data());
    ui->lblCRTC->setText(EmulatorWorkerThread::debugStringCRTC.data());
    ui->lblGateArray->setText(EmulatorWorkerThread::debugStringGateArray.data());
    EmulatorWorkerThread::debugLock.unlock();
    setEnabled(true);
}

void Debugger::onRunClicked()
{
    hide();
    EmulatorWorkerThread::Run();
}

void Debugger::onStepInClicked()
{
    EmulatorWorkerThread::RunStep();
}

void Debugger::onStepOverClicked()
{
    setEnabled(false);
    if (nextInstructionOpCode == 0x18
        || nextInstructionOpCode == 0xC3
        || nextInstructionOpCode == 0xC9
        || nextInstructionOpCode == 0xE9)
        EmulatorWorkerThread::RunStep();
    else
        EmulatorWorkerThread::RunTo(Z80::PC + nextInstructionLength);
}

void Debugger::onStepOutClicked()
{
    setEnabled(false);
    word address = Z80::SP.Get();
    BYTE L = CPC::InternalRAM->MEM[address];
    BYTE H = CPC::InternalRAM->MEM[address + 1];
    EmulatorWorkerThread::RunTo(L + H * 256);
}

void Debugger::onRunToClicked()
{
    setEnabled(false);
    int index = ui->listDisassembly->currentIndex().row();
    QString string = listDisassembly.at(index).mid(18, 4);
    EmulatorWorkerThread::RunTo(string.toInt(nullptr, 16));
}


#include "Debugger.h"
#include "ui_Debugger.h"
#include "EmulatorWorkerThread.h"
#include "Emulator/Headers/Disassembler.h"
#include "Emulator/Headers/Z80.h"
#include "Emulator/Headers/CPC.h"
#include <QCloseEvent>
#include <QStringListModel>
#include <QModelIndex>

Debugger::Debugger(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Debugger)
{
    ui->setupUi(this);
    connect(ui->btnRun, &QPushButton::clicked, this, &Debugger::onRunClicked);
    connect(ui->btnStepIn, &QPushButton::clicked, this, &Debugger::onStepInClicked);
    connect(ui->btnStepOver, &QPushButton::clicked, this, &Debugger::onStepOverClicked);
    connect(ui->btnStepOut, &QPushButton::clicked, this, &Debugger::onStepOutClicked);
    modelDisassembly = new QStringListModel();
    ui->listDisassembly->setModel(modelDisassembly);
    modelMemory = new QStringListModel();
    ui->listMemory->setModel(modelMemory);
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
    listDisassembly.clear();
    char buff[200];
    Disassembler::SetPoint(0x0000);
    while (Disassembler::addr < 0x10000)
    {
        BYTE instrLength;
        string address, bytes, instruction;
        Disassembler::GetNextInstruction(instrLength, &address, &bytes, &instruction);
        sprintf(buff, "%4s  %12s  %s", address.data(), bytes.data(), instruction.data());
        listDisassembly.append(buff);
        if (Disassembler::addr <= Z80::PC)
            pcPosition = listDisassembly.count();
    }
    modelDisassembly -> setStringList(listDisassembly);
    modelDisassembly -> dataChanged(modelDisassembly->index(0x0000), modelDisassembly->index(0xFFFF));
    modelDisassemblyIndex = modelDisassembly->index(pcPosition);
    ui->listDisassembly->scrollTo(modelDisassemblyIndex, QAbstractItemView::PositionAtCenter);
    ui->listDisassembly->setCurrentIndex(modelDisassemblyIndex);

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
    modelMemory -> dataChanged(modelMemory->index(0), modelMemory->index(0xFFFF));
    modelMemoryIndex = modelDisassembly->index(0);
    ui->listMemory->scrollTo(modelMemoryIndex, QAbstractItemView::PositionAtTop);

    EmulatorWorkerThread::debugLock.lock();
    ui->lblZ80->setText(EmulatorWorkerThread::debugStringZ80.data());
    ui->lblStack->setText(EmulatorWorkerThread::debugStringStack.data());
    ui->lblCRTC->setText(EmulatorWorkerThread::debugStringCRTC.data());
    ui->lblGateArray->setText(EmulatorWorkerThread::debugStringGateArray.data());
    EmulatorWorkerThread::debugLock.unlock();
}

void Debugger::onRunClicked()
{
    EmulatorWorkerThread::Run();
    hide();
}

void Debugger::onStepInClicked()
{
    EmulatorWorkerThread::StepIn();
}

void Debugger::onStepOverClicked()
{
    EmulatorWorkerThread::StepOver();
}

void Debugger::onStepOutClicked()
{
    EmulatorWorkerThread::StepOut();
}


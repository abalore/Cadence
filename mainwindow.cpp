#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Debugger.h"
#include "EmulatorThread.h"
#include "Emulator/Headers/Emulator.h"
#include "Emulator/Headers/CPC.h"
#include "Emulator/Headers/Tape.h"
#include "Emulator/Headers/FDC.h"
#include "Emulator/Headers/GateArray.h"
#include <QFrame>
#include <QKeyEvent>
#include <QThread>
#include <QFileDialog>
#include <QFile>
#include <QByteArray>

using namespace std;

MainWindow *MainWindow::Instance;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    debugger = new Debugger(this);
    graphicsInspector = new GraphicsInspector(this);
    enterBytesDialog = new EnterBytesDialog(this);

    Instance = this;
    setFixedSize(800, 600);

    workerThread = new EmulatorThread(this);
    soundThread = new SoundThread(this);

    connect(workerThread, &EmulatorThread::OnPause, this, &MainWindow::onEmulatorPaused);
    connect(workerThread, &EmulatorThread::OnFinishedFrame, this, &MainWindow::onEmulatorFinishedFrame);

    StartThreads();

    connect(ui->actionPause, &QAction::triggered, workerThread, &EmulatorThread::Pause);
    connect(ui->actionReset, &QAction::triggered, this, &MainWindow::ResetEmulation);
    connect(ui->actionLoad_File, &QAction::triggered, this, &MainWindow::onMenuTapeLoadFile);
    connect(ui->actionInspect_video_memory, &QAction::triggered, this, &MainWindow::onMenuScreenInspectGraphics);
    connect(ui->actionSmooth, &QAction::changed, this, &MainWindow::onMenuScreenSmooth);
    connect(ui->actionLoad_DSK, &QAction::triggered, this, &MainWindow::onMenuDiscLoadDSK);
    connect(ui->actionLoad_fromFile, &QAction::triggered, this, &MainWindow::onMenuROMLoadFromFile);
    connect(ui->actionEnter_bytes, &QAction::triggered, this, &MainWindow::onMenuMemoryEnterBytes);
    connect(ui->actionLoad_binary_file, &QAction::triggered, this, &MainWindow::onMenuMemoryLoadBinaryFile);
    connect(ui->actionSave_binary_file, &QAction::triggered, this, &MainWindow::onMenuMemorySaveBinaryFile);
    connect(ui->actionEnable_cartridge, &QAction::changed, this, &MainWindow::onMenuCartridgeEnableCartridge);
    connect(ui->actionLoad_cartridge, &QAction::triggered, this, &MainWindow::onMenuCartridgeLoadCartridge);
    connect(ui->actionGreen_monitor, &QAction::changed, this, &MainWindow::onMenuScreenGreenMonitor);

    connect(ui->actionAmstrad_CPC464, &QAction::triggered, this, &MainWindow::SetCPC464);
    connect(ui->actionAmstrad_CPC664, &QAction::triggered, this, &MainWindow::SetCPC664);
    connect(ui->actionAmstrad_CPC6128, &QAction::triggered, this, &MainWindow::SetCPC6128);
}

MainWindow::~MainWindow()
{
    StopThreads();
    delete graphicsInspector;
    delete debugger;
    delete ui;
}

void MainWindow::StartThreads()
{
    workerThread->end = false;
    soundThread->end = false;
    soundThread->start(QThread::HighPriority);
    workerThread->start(QThread::HighPriority);
}

void MainWindow::StopThreads()
{
    workerThread->end = true;
    soundThread->end = true;
    SoundThread::waitCondition.wakeAll();
    workerThread->wait();
    soundThread->wait();
}

void MainWindow::ResetEmulation()
{
    StopThreads();
    StartThreads();
}

void MainWindow::onMenuMemoryEnterBytes()
{
    enterBytesDialog->show();
}

void MainWindow::onEmulatorPaused()
{
    if (debugger->isHidden())
        debugger->show();
    debugger->Update();
}

void MainWindow::onEmulatorFinishedFrame()
{
    ui->openGLWidget->updateTexture();
}

void MainWindow::onMenuMemoryLoadBinaryFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load binary"), ".", tr("Binary Files (*.bin)"));
    QFile file = QFile(fileName);
    file.open(QIODevice::ReadOnly);
    if (file.isOpen())
    {
        QByteArray ba = file.readAll();
        memcpy(CPC::RAM[1], ba.data(), ba.size());
    }
    file.close();
}

void MainWindow::onMenuMemorySaveBinaryFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save binary"), ".", tr("Binary Files (*.bin)"));
    QFile file = QFile(fileName);
    file.open(QIODevice::WriteOnly);
    if (file.isOpen())
    {
        file.write((const char *)CPC::RAM[1] + 0x3000, 0x1000);
    }
    file.close();
}

void MainWindow::onMenuScreenInspectGraphics()
{
    if (graphicsInspector->isHidden())
        graphicsInspector->show();
    graphicsInspector->UpdateGraphics();
}

void MainWindow::onMenuTapeLoadFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load tape"), ".", tr("Tape Files (*.cdt *.wav)"));
    if (fileName != nullptr)
    {
        QString extension = fileName.last(3);
        if (extension == "wav")
            Tape::LoadWAV((char *)fileName.toUtf8().data());
        else if (extension == "cdt")
            Tape::LoadCDT((char *)fileName.toUtf8().data());
    }
}

void MainWindow::onMenuScreenSmooth()
{
    ui->openGLWidget->setSmoothing(ui->actionSmooth->isChecked());
}

void MainWindow::onMenuDiscLoadDSK()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load disc"), ".", tr("DSK Files (*.dsk)"));
    if (fileName != nullptr)
        if (FDC::GetDrive(0)->InsertDSK((char *)fileName.toUtf8().data())) {}
}

void MainWindow::onMenuROMLoadFromFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load ROM"), ".", tr("ROM Files (*.bin *.rom)"));
    if (fileName != nullptr)
        CPC::ReadROM((char *)fileName.toUtf8().data(), 1);
}

void MainWindow::onMenuCartridgeEnableCartridge()
{
    CPC::cartridgeEnabled = ui->actionEnable_cartridge->isChecked();
    Emulator::Reset();
}

void MainWindow::onMenuCartridgeLoadCartridge()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load Cartridge"), ".", tr("Cartridge Files (*.cpr *.bin *.CPR *.BIN)"));
    if (fileName != nullptr)
        CPC::ReadCartridge((char *)fileName.toUtf8().data());
}

void MainWindow::onMenuScreenGreenMonitor()
{
    GateArray::Monochrome = ui->actionGreen_monitor->isChecked();
}

void MainWindow::SetCPC464()
{
    CPC::cpcType = CPCType::CPC464;
    ResetEmulation();
}

void MainWindow::SetCPC664()
{
    CPC::cpcType = CPCType::CPC664;
    ResetEmulation();
}

void MainWindow::SetCPC6128()
{
    CPC::cpcType = CPCType::CPC6128;
    ResetEmulation();
}

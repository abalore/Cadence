#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Debugger.h"
#include "EmulatorWorkerThread.h"
#include "Emulator/Headers/CPC.h"
#include "Emulator/Headers/CRTScreen.h"
#include "Emulator/Headers/Tape.h"
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
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 1024, 312*2);
    ui->graphicsView->setScene(scene);
    pixItem = 0;
    workerThread = new EmulatorWorkerThread(NULL);
    connect(workerThread, &EmulatorWorkerThread::OnPause, this, &MainWindow::onEmulatorPaused);
    connect(workerThread, &EmulatorWorkerThread::OnFinishedFrame, this, &MainWindow::onEmulatorFinishedFrame);
    workerThread->start(QThread::HighPriority);
    soundThread = new SoundThread(NULL);
    soundThread->start(QThread::HighPriority);
    debugger = new Debugger(this);
    connect(ui->actionPause, &QAction::triggered, this, &MainWindow::onMenuDebugPause);
    connect(ui->actionReset, &QAction::triggered, this, &MainWindow::onMenuDebugReset);
    connect(ui->actionLoad_binary, &QAction::triggered, this, &MainWindow::onMenuFileLoadBinary);
    connect(ui->actionLoad_from_file, &QAction::triggered, this, &MainWindow::onMenuTapeLoadFromFile);
    connect(ui->actionFrom_audio_input, &QAction::triggered, this, &MainWindow::onMenuTapeFromAudioInput);
    Instance = this;
}

MainWindow::~MainWindow()
{
    workerThread->end = true;
    soundThread->end = true;
    while(!workerThread->isFinished() || !soundThread->isFinished()){}
    delete debugger;
    delete ui;
}

void MainWindow::onEmulatorPaused()
{
    if (debugger->isHidden())
        debugger->show();
    debugger->Update();
}

void MainWindow::onMenuFileLoadBinary()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load binary"), ".", tr("Binary Files (*.bin)"));
    QFile file = QFile(fileName);
    file.open(QIODevice::ReadOnly);
    if (file.isOpen())
    {
        QByteArray ba = file.readAll();
        memcpy(CPC::InternalRAM->MEM + 0x100, ba.data(), ba.size());
    }
    file.close();
}

void MainWindow::onMenuDebugPause()
{
    EmulatorWorkerThread::RunStep();
}

void MainWindow::onMenuDebugReset()
{
    EmulatorWorkerThread::Reset();
}

void MainWindow::onEmulatorFinishedFrame()
{
    if (pixItem) scene->removeItem(pixItem);
    QImage *image = new QImage(&CRTScreen::Pixels[0], 1024, 624, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(*image, Qt::NoFormatConversion | Qt::NoOpaqueDetection);
    pixItem = scene->addPixmap(pixmap);
    pixItem->setPos(-64, -20);
}

void MainWindow::onMenuTapeLoadFromFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load tape"), ".", tr("WAV Files (*.wav)"));
    Tape::LoadWAV((char *)fileName.toUtf8().data());
}

void MainWindow::onMenuTapeFromAudioInput()
{
    Tape::FromAudioInput(ui->actionFrom_audio_input->isChecked());
}

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Debugger.h"
#include "EmulatorWorkerThread.h"
#include "Emulator/Headers/CPC.h"
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

    workerThread = new EmulatorWorkerThread(NULL);
    connect(workerThread, &EmulatorWorkerThread::OnPause, this, &MainWindow::onEmulatorPaused);
    connect(workerThread, &EmulatorWorkerThread::OnFinishedFrame, this, &MainWindow::onEmulatorFinishedFrame);
    workerThread->start(QThread::HighPriority);
    soundThread = new SoundThread(NULL);
    soundThread->start(QThread::HighPriority);

    debugger = new Debugger(this);
    graphicsInspector = new GraphicsInspector(this);

    connect(ui->actionPause, &QAction::triggered, this, &MainWindow::onMenuDebugPause);
    connect(ui->actionReset, &QAction::triggered, this, &MainWindow::onMenuDebugReset);
    connect(ui->actionLoad_binary, &QAction::triggered, this, &MainWindow::onMenuFileLoadBinary);
    connect(ui->actionLoad_WAV, &QAction::triggered, this, &MainWindow::onMenuTapeLoadWAV);
    connect(ui->actionLoad_CDT, &QAction::triggered, this, &MainWindow::onMenuTapeLoadCDT);
    connect(ui->actionInspect_graphics, &QAction::triggered, this, &MainWindow::onMenuDebugInspectGraphics);

    Instance = this;

    //screenView->show();

    setFixedSize(768, 624);

    startTimer(1000, Qt::CoarseTimer);
}

MainWindow::~MainWindow()
{
    workerThread->end = true;
    soundThread->end = true;
    while(!workerThread->isFinished() || !soundThread->isFinished()){}
    delete graphicsInspector;
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
    EmulatorWorkerThread::Pause();
}

void MainWindow::onMenuDebugReset()
{
    EmulatorWorkerThread::Reset();
}

void MainWindow::onMenuDebugInspectGraphics()
{
    if (graphicsInspector->isHidden())
        graphicsInspector->show();
    graphicsInspector->UpdateGraphics();
}

void MainWindow::onEmulatorFinishedFrame()
{
    ui->openGLWidget->updateTexture();
}

void MainWindow::onMenuTapeLoadWAV()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load tape"), ".", tr("WAV Files (*.wav)"));
    if (fileName != nullptr)
        Tape::LoadWAV((char *)fileName.toUtf8().data());
}

void MainWindow::onMenuTapeLoadCDT()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load tape"), ".", tr("CDT Files (*.cdt)"));
    if (fileName != nullptr)
        Tape::LoadCDT((char *)fileName.toUtf8().data());
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    event->accept();
    QString str = QString::number(SoundThread::lastElapsed);
    ui->label->setText(str);
}

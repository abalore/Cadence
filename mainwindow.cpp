#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Debugger.h"
#include "EmulatorWorkerThread.h"
#include "speedcontroller.h"
#include "Emulator/Headers/CPC.h"
#include "Emulator/Headers/Tape.h"
#include "Emulator/Headers/FDC.h"
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

    Instance = this;
    //screenView->show();
    //setFixedSize(768, 624);

    workerThread = new EmulatorWorkerThread(this);
    soundThread = new SoundThread(this);

    connect(workerThread, &EmulatorWorkerThread::OnPause, this, &MainWindow::onEmulatorPaused);
    connect(workerThread, &EmulatorWorkerThread::OnFinishedFrame, this, &MainWindow::onEmulatorFinishedFrame);

    soundThread->start(QThread::HighPriority);
    workerThread->start(QThread::HighPriority);

    connect(ui->actionPause, &QAction::triggered, workerThread, &EmulatorWorkerThread::Pause);
    connect(ui->actionReset, &QAction::triggered, workerThread, &EmulatorWorkerThread::Reset);
    connect(ui->actionLoad_binary, &QAction::triggered, this, &MainWindow::onMenuFileLoadBinary);
    connect(ui->actionLoad_File, &QAction::triggered, this, &MainWindow::onMenuTapeLoadFile);
    connect(ui->actionInspect_video_memory, &QAction::triggered, this, &MainWindow::onMenuScreenInspectGraphics);
    connect(ui->actionSmooth, &QAction::changed, this, &MainWindow::onMenuScreenSmooth);
    connect(ui->actionLoad_DSK, &QAction::triggered, this, &MainWindow::onMenuDiscLoadDSK);

}

MainWindow::~MainWindow()
{
    workerThread->end = true;
    soundThread->end = true;
    SoundThread::waitCondition.wakeAll();
    workerThread->wait();
    soundThread->wait();
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

void MainWindow::onEmulatorFinishedFrame()
{
    ui->openGLWidget->updateTexture();
}

void MainWindow::onMenuFileLoadBinary()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load binary"), ".", tr("Binary Files (*.bin)"));
    QFile file = QFile(fileName);
    file.open(QIODevice::ReadOnly);
    if (file.isOpen())
    {
        QByteArray ba = file.readAll();
        memcpy(CPC::BaseRAM.MEM + 0x100, ba.data(), ba.size());
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
        if (FDC::GetDrive(0)->InsertDSK((char *)fileName.toUtf8().data()))
        {

        }
}

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Debugger.h"
#include "EmulatorWorkerThread.h"
#include "Emulator/Headers/Z80.h"
#include "Emulator/Headers/CPC.h"
#include "Emulator/Headers/Emulator.h"
#include "Emulator/Headers/CRTScreen.h"
#include <QFrame>
#include <QKeyEvent>
#include <QThread>
#include <QFileDialog>
#include <QFile>
#include <QByteArray>

using namespace std;

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
    workerThread->start();
    workerThread->setPriority(QThread::HighPriority);
    debugger = new Debugger(this);
    connect(ui->actionPause, &QAction::triggered, this, &MainWindow::onMenuDebugPause);
    connect(ui->actionReset, &QAction::triggered, this, &MainWindow::onMenuDebugReset);
    connect(ui->actionLoad_binary, &QAction::triggered, this, &MainWindow::onMenuFileLoadBinary);
    startTimer(17, Qt::PreciseTimer);
}

MainWindow::~MainWindow()
{
    workerThread->end = true;
    while(!workerThread->isFinished()){}
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
    QFile bin = QFile(fileName);
    bin.open(QIODevice::ReadOnly);
    if (bin.isOpen())
    {
        QByteArray ba = bin.readAll();
        memcpy(CPC::InternalRAM->MEM + 0x100, ba.data(), ba.size());
    }
}

void MainWindow::onMenuDebugPause()
{
    EmulatorWorkerThread::RunStep();
}

void MainWindow::onMenuDebugReset()
{
    EmulatorWorkerThread::Reset();
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    event->accept();
    if (pixItem) scene->removeItem(pixItem);
    QImage *image = new QImage(&CRTScreen::Pixels[0], 1024, 624, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(*image, Qt::NoFormatConversion | Qt::NoOpaqueDetection);
    pixItem = scene->addPixmap(pixmap);
    pixItem->setPos(-64, -20);
    //EmulatorWorkerThread::measures++;
    //EmulatorWorkerThread::total += EmulatorWorkerThread::iteration;
    //EmulatorWorkerThread::iteration = 0;
    ui->label->setText(QString::number(EmulatorWorkerThread::elapsed, 10));
}

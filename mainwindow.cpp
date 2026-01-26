#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Debugger.h"
#include "EmulatorWorkerThread.h"
#include "Emulator/Headers/CRTScreen.h"
#include <QFrame>
#include <QKeyEvent>
#include <QThread>

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
    connect(ui->menuDebug, &QMenu::triggered, this, &MainWindow::onMenuDebugPause);
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

void MainWindow::onMenuDebugPause()
{
    EmulatorWorkerThread::Pause();
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    event->accept();
    if (pixItem) scene->removeItem(pixItem);
    QImage *image = new QImage(&CRTScreen::Pixels[0], 1024, 624, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(*image, Qt::NoFormatConversion | Qt::NoOpaqueDetection);
    pixItem = scene->addPixmap(pixmap);
    pixItem->setPos(-96, -24);
    EmulatorWorkerThread::measures++;
    EmulatorWorkerThread::total += EmulatorWorkerThread::iteration;
    EmulatorWorkerThread::iteration = 0;
    ui->label->setText(QString::number(EmulatorWorkerThread::total / EmulatorWorkerThread::measures, 10));
}

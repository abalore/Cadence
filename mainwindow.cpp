#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "EmulatorWorkerThread.h"
#include "Emulator/Headers/CRTScreen.h"
#include <QFrame>
#include <QKeyEvent>

#include <mutex>

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->btnRun, &QPushButton::clicked, this,&MainWindow::onRunClicked);
    connect(ui->btnPause, &QPushButton::clicked, this,&MainWindow::onPauseClicked);
    connect(ui->btnStepIn, &QPushButton::clicked, this,&MainWindow::on_pushButton_clicked);
    connect(ui->btnStepOver, &QPushButton::clicked, this,&MainWindow::onStepOverClicked);
    connect(ui->btnStepOut, &QPushButton::clicked, this,&MainWindow::onStepOutClicked);

    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 1024, 312*2);
    ui->graphicsView->setScene(scene);

    pixItem = 0;

    EmulatorWorkerThread *workerThread = new EmulatorWorkerThread(NULL);
    connect(workerThread, &EmulatorWorkerThread::OnPause, this, &MainWindow::onEmulatorPaused);
    workerThread->start();
    workerThread->setPriority(QThread::HighPriority);

    startTimer(17, Qt::PreciseTimer);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetDebugState(bool state)
{
    ui->btnStepIn->setEnabled(state);
    ui->btnStepOver->setEnabled(state);
    ui->btnStepOut->setEnabled(state);
    ui->btnRun->setEnabled(state);
}

void MainWindow::onEmulatorPaused()
{
    EmulatorWorkerThread::debugLock.lock();
    ui->lblZ80->setText(EmulatorWorkerThread::debugStringZ80.data());
    ui->lblStack->setText(EmulatorWorkerThread::debugStringStack.data());
    ui->lblDisassembler->setText(EmulatorWorkerThread::debugStringDisassembler.data());
    ui->lblCRTC->setText(EmulatorWorkerThread::debugStringCRTC.data());
    ui->lblGateArray->setText(EmulatorWorkerThread::debugStringGateArray.data());
    ui->lblMem->setText(EmulatorWorkerThread::debugStringMem.data());
    SetDebugState(true);
    EmulatorWorkerThread::debugLock.unlock();
}

void MainWindow::onRunClicked()
{
    EmulatorWorkerThread::Run();
    SetDebugState(false);
}

void MainWindow::onPauseClicked()
{
    EmulatorWorkerThread::Pause();
    SetDebugState(true);
}

void MainWindow::on_pushButton_clicked()
{
    EmulatorWorkerThread::StepIn();
    SetDebugState(false);
}

void MainWindow::onStepOverClicked()
{
    EmulatorWorkerThread::StepOver();
    SetDebugState(false);
}

void MainWindow::onStepOutClicked()
{
    EmulatorWorkerThread::StepOut();
    SetDebugState(false);
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    event->accept();
    onRenderClicked();
}

void MainWindow::onRenderClicked()
{
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

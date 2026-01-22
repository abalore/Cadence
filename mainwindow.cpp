#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "EmulatorWorkerThread.h"
#include "Emulator/Headers/CRTScreen.h"

#include <QFrame>

// #include <mutex>

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->btnStepIn, &QPushButton::clicked, this,&MainWindow::on_pushButton_clicked);
    connect(ui->btnStepOver, &QPushButton::clicked, this,&MainWindow::onStepOverClicked);
    connect(ui->btnStepOut, &QPushButton::clicked, this,&MainWindow::onStepOutClicked);
    connect(ui->btnRender, &QPushButton::clicked, this,&MainWindow::onRenderClicked);

    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 1024, 312*2);
    ui->graphicsView->setScene(scene);

    pixItem = 0;

    EmulatorWorkerThread *workerThread = new EmulatorWorkerThread(NULL);
    connect(workerThread, &EmulatorWorkerThread::OnPause, this, &MainWindow::onEmulatorPaused);
    workerThread->start();

    startTimer(20, Qt::PreciseTimer);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::DisableDebugButtons()
{
    ui->btnStepIn->setEnabled(false);
    ui->btnStepOver->setEnabled(false);
    ui->btnStepOut->setEnabled(false);
}

void MainWindow::EnableDebugButtons()
{
    ui->btnStepIn->setEnabled(true);
    ui->btnStepOver->setEnabled(true);
    ui->btnStepOut->setEnabled(true);
}

void MainWindow::onEmulatorPaused()
{
    //Z80::debugStringLock.lock();
    ui->lblZ80->setText(EmulatorWorkerThread::debugStringZ80.data());
    ui->lblStack->setText(EmulatorWorkerThread::debugStringStack.data());
    ui->lblDisassembler->setText(EmulatorWorkerThread::debugStringDisassembler.data());
    ui->lblCRTC->setText(EmulatorWorkerThread::debugStringCRTC.data());
    ui->lblGateArray->setText(EmulatorWorkerThread::debugStringGateArray.data());
    EnableDebugButtons();

    //Z80::debugStringLock.unlock();
}

void MainWindow::on_pushButton_clicked()
{
    EmulatorWorkerThread::StepIn();
    DisableDebugButtons();
}

void MainWindow::onStepOverClicked()
{
    EmulatorWorkerThread::StepOver();
    DisableDebugButtons();
}

void MainWindow::onStepOutClicked()
{
    EmulatorWorkerThread::StepOut();
    DisableDebugButtons();
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    onRenderClicked();
}

void MainWindow::onRenderClicked()
{
    if (pixItem) scene->removeItem(pixItem);
    QImage *image = new QImage(&CRTScreen::Pixels[0], 1024, 624, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(*image, Qt::NoFormatConversion | Qt::NoOpaqueDetection);
    pixItem = scene->addPixmap(pixmap);
}

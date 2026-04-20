#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Debugger.h"
#include "EmulatorThread.h"
#include "Emulator.h"
#include "CPC.h"
#include "Tape.h"
#include "FDC.h"
#include "GateArray.h"
#include "CRTScreen.h"
#include "Keyboard.h"
#include <QFrame>
#include <QKeyEvent>
#include <QThread>
#include <QFileDialog>
#include <QFile>
#include <QByteArray>
#include <QWindow>
#include <QInputDialog>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QActionGroup>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

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
    //setFixedSize(1024, 768);

    workerThread = new EmulatorThread(this);
    soundThread = new SoundThread(this);

    connect(workerThread, &EmulatorThread::OnPause, this, &MainWindow::onEmulatorPaused);
    connect(workerThread, &EmulatorThread::OnResume, this, &MainWindow::onEmulatorResumed);
    connect(workerThread, &EmulatorThread::OnFinishedFrame, this, &MainWindow::onEmulatorFinishedFrame);

    StartThreads();

    connect(ui->actionPause, &QAction::triggered, workerThread, &EmulatorThread::Pause);
    connect(ui->actionReset, &QAction::triggered, this, &MainWindow::ResetEmulation);
    connect(ui->actionInsert_tape, &QAction::triggered, this, &MainWindow::onMenuMediaInsertTape);
    connect(ui->actionInspect_video_memory, &QAction::triggered, this, &MainWindow::onMenuScreenInspectGraphics);
    connect(ui->actionSmooth, &QAction::changed, this, &MainWindow::onMenuScreenSmooth);
    connect(ui->actionInsert_disk, &QAction::triggered, this, &MainWindow::onMenuMediaInsertDiskA);
    connect(ui->actionROM_Box, &QAction::triggered, this, &MainWindow::onMenuROMLoadFromFile);
    connect(ui->actionEnter_bytes, &QAction::triggered, this, &MainWindow::onMenuMemoryEnterBytes);
    connect(ui->actionLoad_binary_file, &QAction::triggered, this, &MainWindow::onMenuMemoryLoadBinaryFile);
    connect(ui->actionSave_binary_file, &QAction::triggered, this, &MainWindow::onMenuMemorySaveBinaryFile);
    connect(ui->actionRemove_cartridge, &QAction::triggered, this, &MainWindow::onMenuMediaRemoveCartridge);
    connect(ui->actionInsert_cartridge, &QAction::triggered, this, &MainWindow::onMenuMediaInsertCartridge);
    connect(ui->actionGreen_monitor, &QAction::changed, this, &MainWindow::onMenuScreenGreenMonitor);
    connect(ui->actionFull_screen, &QAction::changed, this, &MainWindow::onMenuViewFullScreen);

    QActionGroup *modelGroup = new QActionGroup(this);
    modelGroup->setExclusive(true);
    modelGroup->addAction(ui->actionAmstrad_CPC464);
    modelGroup->addAction(ui->actionAmstrad_CPC664);
    modelGroup->addAction(ui->actionAmstrad_CPC6128);

    connect(ui->actionAmstrad_CPC464, &QAction::triggered, this, &MainWindow::SetCPC464);
    connect(ui->actionAmstrad_CPC664, &QAction::triggered, this, &MainWindow::SetCPC664);
    connect(ui->actionAmstrad_CPC6128, &QAction::triggered, this, &MainWindow::SetCPC6128);
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionAudio_enabled, &QAction::toggled, this, [](bool checked){ SoundThread::enabled = checked; });
    connect(ui->actionRight_shift_as_backslash, &QAction::toggled, this, [](bool checked){ Keyboard::translation[53] = checked ? 62 : 52; });
    connect(ui->actionAbout, &QAction::triggered, this, [this]{
        QDialog dialog(this);
        dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        dialog.setStyleSheet(
            "QDialog { background-color: #0b1220; border: 1px solid #00e5ff; }"
            "QLabel { color: #e0e8f0; background: transparent; }"
            "QPushButton { background-color: #0b1220; color: #00e5ff; "
            "border: 1px solid #00e5ff; padding: 6px 28px; border-radius: 4px; font-weight: bold; }"
            "QPushButton:hover { background-color: #00e5ff; color: #0b1220; }"
            "QPushButton:pressed { background-color: #00b8cc; color: #0b1220; }"
        );

        QVBoxLayout *layout = new QVBoxLayout(&dialog);
        layout->setContentsMargins(40, 28, 40, 24);
        layout->setSpacing(10);

        QLabel *iconLabel = new QLabel(&dialog);
        iconLabel->setPixmap(QIcon(":/images/cadence.svg").pixmap(80, 80));
        iconLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(iconLabel, 0, Qt::AlignHCenter);

        QLabel *titleLabel = new QLabel(QString("%1 %2").arg(APP_NAME, APP_VERSION), &dialog);
        QFont titleFont = titleLabel->font();
        titleFont.setBold(true);
        titleFont.setPointSize(titleFont.pointSize() + 5);
        titleLabel->setFont(titleFont);
        titleLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(titleLabel);

        QLabel *copyLabel = new QLabel("(c) Abalore 2026", &dialog);
        QFont copyFont = copyLabel->font();
        copyFont.setPointSize(copyFont.pointSize() - 1);
        copyLabel->setFont(copyFont);
        copyLabel->setAlignment(Qt::AlignCenter);
        copyLabel->setStyleSheet("QLabel { color: #8a96a8; }");
        layout->addWidget(copyLabel);

        layout->addSpacing(12);

        QPushButton *okButton = new QPushButton("OK", &dialog);
        okButton->setDefault(true);
        okButton->setFocusPolicy(Qt::StrongFocus);
        connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
        layout->addWidget(okButton, 0, Qt::AlignHCenter);

        dialog.exec();
    });

    ui->hLine->setVisible(false);
    ui->vLine->setVisible(false);
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
    ui->hLine->setVisible(true);
    ui->vLine->setVisible(true);
    ui->hLine->move(0, CRTScreen::vPos * 2 - 56);
    ui->vLine->move(CRTScreen::hPos + 240, 0);
    ui->vLine->pos().setX(CRTScreen::vPos);
    if (debugger->isHidden())
        debugger->show();
    debugger->Update();
}

void MainWindow::onEmulatorResumed()
{
    ui->hLine->setVisible(false);
    ui->vLine->setVisible(false);
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
        file.close();
        bool ok;
        QString addressText = QInputDialog::getText(this, tr("Target address"), tr("Target address (hex):"), QLineEdit::Normal, "4000", &ok);
        if (ok)
        {
            word address = addressText.toUShort(nullptr, 16);
            for (int i = 0; i < ba.size(); i++)
                CPC::SetByteAt(address + i, (BYTE)ba[i]);
        }
    }
}

void MainWindow::onMenuMemorySaveBinaryFile()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Save binary"));
    QFormLayout *layout = new QFormLayout(&dialog);
    QLineEdit *addressEdit = new QLineEdit("0000", &dialog);
    QLineEdit *lengthEdit = new QLineEdit("4000", &dialog);
    layout->addRow(tr("Source address (hex):"), addressEdit);
    layout->addRow(tr("Length (hex):"), lengthEdit);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) return;
    word address = addressEdit->text().toUShort(nullptr, 16);
    int length = lengthEdit->text().toInt(nullptr, 16);
    if (length <= 0) return;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save binary"), ".", tr("Binary Files (*.bin)"));
    QFile file = QFile(fileName);
    file.open(QIODevice::WriteOnly);
    if (file.isOpen())
    {
        QByteArray ba;
        ba.resize(length);
        for (int i = 0; i < length; i++)
            ba[i] = (char)CPC::GetByteAt(address + i);
        file.write(ba);
        file.close();
    }
}

void MainWindow::onMenuScreenInspectGraphics()
{
    if (graphicsInspector->isHidden())
        graphicsInspector->show();
    graphicsInspector->UpdateGraphics();
}

void MainWindow::onMenuMediaInsertTape()
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

void MainWindow::onMenuMediaInsertDiskA()
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

void MainWindow::onMenuMediaRemoveCartridge()
{
    CPC::cartridgeEnabled = false;
    Emulator::Reset();
}

void MainWindow::onMenuMediaInsertCartridge()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load Cartridge"), ".", tr("Cartridge Files (*.cpr *.bin *.CPR *.BIN)"));
    if (fileName != nullptr)
        CPC::ReadCartridge((char *)fileName.toUtf8().data());
    CPC::cartridgeEnabled = true;
    Emulator::Reset();
}

void MainWindow::onMenuScreenGreenMonitor()
{
    GateArray::Monochrome = ui->actionGreen_monitor->isChecked();
}

void MainWindow::onMenuViewFullScreen()
{
    QSize size = QSize(768, 544);
    if (ui->actionFull_screen->isChecked())
    {
        float ratio = (float)size.height() / size.width();
        QScreen *screen = this->windowHandle()->screen();
        showFullScreen();
        this->menuBar()->setFixedHeight(0);
        float height = screen->availableSize().height();
        ui->openGLWidget->setFixedSize(QSize(height / ratio, height));
        ui->centralwidget->setStyleSheet("background-color:black;");
    }
    else
    {
        this->menuBar()->setFixedHeight(23);
        showNormal();
        ui->openGLWidget->setFixedSize(size);
        ui->centralwidget->setStyleSheet("background-color:gray;");
    }
}

void MainWindow::SwitchMachine(CPCType type)
{
    StopThreads();
    Emulator::Finalize();
    CPC::cpcType = type;
    Emulator::Init();
    StartThreads();
}

void MainWindow::SetCPC464()
{
    SwitchMachine(CPCType::CPC464);
}

void MainWindow::SetCPC664()
{
    SwitchMachine(CPCType::CPC664);
}

void MainWindow::SetCPC6128()
{
    SwitchMachine(CPCType::CPC6128);
}

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Debugger.h"
#include "EmulatorThread.h"
#include "CPC.h"
#include "Tape.h"
#include "FDC.h"
#include "GateArray.h"
#include "CRTScreen.h"
#include "Keyboard.h"
#include "AboutDialog.h"
#include "ROMBoxDialog.h"
#include <QFrame>
#include <QKeyEvent>
#include <QThread>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QByteArray>
#include <QWindow>
#include <QInputDialog>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QActionGroup>
#include <QMessageBox>
#include <QLabel>
#include <QStatusBar>
#include <QFileInfo>
#include <QIcon>
#include <QFontMetrics>
#include <QTimer>

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

    settings.Load();
    if (settings.system == "CPC464")       CPC::cpcType = CPCType::CPC464;
    else if (settings.system == "CPC664")  CPC::cpcType = CPCType::CPC664;
    else                                   CPC::cpcType = CPCType::CPC6128;

    workerThread = new EmulatorThread(this);
    soundThread = new SoundThread(this);
    media = new MediaController(this);
    applyROMOverrides();

    connect(workerThread, &EmulatorThread::OnPause, this, &MainWindow::onEmulatorPaused);
    connect(workerThread, &EmulatorThread::OnResume, this, &MainWindow::onEmulatorResumed);
    connect(workerThread, &EmulatorThread::OnFinishedFrame, this, &MainWindow::onEmulatorFinishedFrame);
    connect(media, &MediaController::mediaChanged, this, &MainWindow::onMediaChanged);

    connect(ui->actionPause, &QAction::triggered, workerThread, &EmulatorThread::Pause);
    connect(ui->actionReset, &QAction::triggered, this, &MainWindow::ResetEmulation);
    connect(ui->actionInsert_tape, &QAction::triggered, this, &MainWindow::onMenuMediaInsertTape);
    connect(ui->actionRemove_tape, &QAction::triggered, this, &MainWindow::onMenuMediaRemoveTape);
    connect(ui->actionRemove_disk, &QAction::triggered, this, &MainWindow::onMenuMediaRemoveDiskA);
    connect(ui->actionInsert_disk_2, &QAction::triggered, this, &MainWindow::onMenuMediaInsertDiskB);
    connect(ui->actionRemove_disk_2, &QAction::triggered, this, &MainWindow::onMenuMediaRemoveDiskB);
    connect(ui->actionInspect_video_memory, &QAction::triggered, this, &MainWindow::onMenuScreenInspectGraphics);
    connect(ui->actionSmooth, &QAction::changed, this, &MainWindow::onMenuScreenSmooth);
    connect(ui->actionInsert_disk, &QAction::triggered, this, &MainWindow::onMenuMediaInsertDiskA);
    connect(ui->actionNew_blank_disk, &QAction::triggered, this, &MainWindow::onMenuMediaNewBlankDiskA);
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
    connect(ui->actionSFX_enabled, &QAction::toggled, this, [](bool checked){ SoundThread::sfxEnabled = checked; });
    connect(ui->actionTape_enabled, &QAction::toggled, this, [](bool checked){ CPC::tape.audioEnabled = checked; });
    connect(ui->actionRight_shift_as_backslash, &QAction::toggled, this, [](bool checked){ Keyboard::translation[53] = checked ? 62 : 52; });
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onMenuAbout);

    ui->hLine->setVisible(false);
    ui->vLine->setVisible(false);

    statusBar()->setSizeGripEnabled(false);

    const int iconSize = 24;
    const int sectionWidth = 771 / 5;
    const int textWidth = sectionWidth - iconSize - 6;
    auto leftStretch = new QWidget(this);
    leftStretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    statusBar()->addWidget(leftStretch, 1);
    auto addMediaLabel = [this](const QString &iconPath, QLabel *&textLabel) {
        QLabel *icon = new QLabel(this);
        icon->setPixmap(QIcon(iconPath).pixmap(iconSize, iconSize));
        textLabel = new QLabel(this);
        textLabel->setFixedWidth(textWidth);
        textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        statusBar()->addWidget(icon);
        statusBar()->addWidget(textLabel);
        setMediaText(textLabel, "<Empty>");
    };
    addMediaLabel(":/images/cartridge.png", cartridgeLabel);
    addMediaLabel(":/images/tape.png", tapeLabel);
    addMediaLabel(":/images/disk.png", diskLabel);
    addMediaLabel(":/images/disk.png", diskBLabel);
    ledOnPixmap = QIcon(":/images/led_on.png").pixmap(iconSize, iconSize / 2);
    ledOffPixmap = QIcon(":/images/led_off.png").pixmap(iconSize, iconSize / 2);
    motorLabel = new QLabel(this);
    motorLabel->setPixmap(ledOffPixmap);
    statusBar()->addWidget(motorLabel);
    auto rightStretch = new QWidget(this);
    rightStretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    statusBar()->addWidget(rightStretch, 1);

    ui->centralwidget->setFixedSize(771, 547);
    adjustSize();
    setFixedSize(size());
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint
                   | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint
                   | Qt::WindowMinimizeButtonHint);

    applySettingsToUi();
    if (!settings.diskAPath.isEmpty() && QFileInfo::exists(settings.diskAPath))
        media->LoadDiskA(settings.diskAPath);
    if (!settings.diskBPath.isEmpty() && QFileInfo::exists(settings.diskBPath))
        media->LoadDiskB(settings.diskBPath);
    if (!settings.tapePath.isEmpty() && QFileInfo::exists(settings.tapePath))
        media->LoadTape(settings.tapePath);
    if (!settings.cartridgePath.isEmpty() && QFileInfo::exists(settings.cartridgePath))
        media->LoadCartridge(settings.cartridgePath);

    StartThreads();
}

MainWindow::~MainWindow()
{
    collectSettingsFromUi();
    settings.diskAPath     = media->DiskAPath();
    settings.diskBPath     = media->DiskBPath();
    settings.tapePath      = media->TapePath();
    settings.cartridgePath = media->CartridgePath();
    settings.Save();
    StopThreads();
    delete graphicsInspector;
    delete debugger;
    delete ui;
}

void MainWindow::applySettingsToUi()
{
    ui->actionSmooth->setChecked(settings.smooth);
    ui->actionGreen_monitor->setChecked(settings.greenMonitor);
    ui->actionAudio_enabled->setChecked(settings.audioEnabled);
    ui->actionSFX_enabled->setChecked(settings.sfxEnabled);
    ui->actionTape_enabled->setChecked(settings.tapeEnabled);
    ui->actionRight_shift_as_backslash->setChecked(settings.rsBackslash);
    switch (CPC::cpcType) {
    case CPCType::CPC464:  ui->actionAmstrad_CPC464->setChecked(true);  break;
    case CPCType::CPC664:  ui->actionAmstrad_CPC664->setChecked(true);  break;
    case CPCType::CPC6128: ui->actionAmstrad_CPC6128->setChecked(true); break;
    }

    const bool smooth = settings.smooth;
    QTimer::singleShot(0, this, [this, smooth]{ ui->openGLWidget->setSmoothing(smooth); });
    CPC::gateArray.SetMonochrome(settings.greenMonitor);
    SoundThread::enabled = settings.audioEnabled;
    SoundThread::sfxEnabled = settings.sfxEnabled;
    CPC::tape.audioEnabled = settings.tapeEnabled;
    Keyboard::translation[53] = settings.rsBackslash ? 62 : 52;

    if (settings.fullScreen)
        QTimer::singleShot(0, this, [this]{ ui->actionFull_screen->setChecked(true); });
}

void MainWindow::collectSettingsFromUi()
{
    settings.smooth       = ui->actionSmooth->isChecked();
    settings.greenMonitor = ui->actionGreen_monitor->isChecked();
    settings.fullScreen   = ui->actionFull_screen->isChecked();
    settings.audioEnabled = ui->actionAudio_enabled->isChecked();
    settings.sfxEnabled   = ui->actionSFX_enabled->isChecked();
    settings.tapeEnabled  = ui->actionTape_enabled->isChecked();
    settings.rsBackslash  = ui->actionRight_shift_as_backslash->isChecked();
}

void MainWindow::onMediaChanged(MediaSlot slot, const QString &text)
{
    switch (slot) {
    case MediaSlot::Tape:      setMediaText(tapeLabel, text);      break;
    case MediaSlot::DiskA:     setMediaText(diskLabel, text);      break;
    case MediaSlot::DiskB:     setMediaText(diskBLabel, text);     break;
    case MediaSlot::Cartridge: setMediaText(cartridgeLabel, text); break;
    }
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
    ui->hLine->move(0, CPC::screen.vPos * 2 - 56);
    ui->vLine->move(CPC::screen.hPos + 240, 0);
    ui->vLine->pos().setX(CPC::screen.vPos);
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
    motorLabel->setPixmap(CPC::fdc.GetState() != FDC_StateCommand ? ledOnPixmap : ledOffPixmap);
}

void MainWindow::onMenuMemoryLoadBinaryFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load binary"), QDir::homePath() + "/.cadence/BIN", tr("Binary Files (*.bin)"));
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

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save binary"), QDir::homePath() + "/.cadence/BIN", tr("Binary Files (*.bin)"));
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
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load tape"), QDir::homePath() + "/.cadence/CDT", tr("Tape Files (*.cdt *.wav)"));
    if (fileName != nullptr)
        media->LoadTape(fileName);
}

void MainWindow::onMenuScreenSmooth()
{
    ui->openGLWidget->setSmoothing(ui->actionSmooth->isChecked());
}

void MainWindow::onMenuMediaInsertDiskA()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load disc"), QDir::homePath() + "/.cadence/DSK", tr("DSK Files (*.dsk)"));
    if (fileName != nullptr)
        media->LoadDiskA(fileName);
}

void MainWindow::onMenuMediaNewBlankDiskA()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("New blank disc"), QDir::homePath() + "/.cadence/DSK", tr("DSK Files (*.dsk)"));
    if (fileName.isEmpty())
        return;
    if (!fileName.endsWith(".dsk", Qt::CaseInsensitive))
        fileName += ".dsk";

    QDialog dlg(this);
    dlg.setWindowTitle(tr("New blank disc"));

    auto makeGroup = [&](const QString &title, const QStringList &labels, int def) {
        QGroupBox *g = new QGroupBox(title, &dlg);
        QHBoxLayout *gl = new QHBoxLayout(g);
        QList<QRadioButton *> btns;
        for (int i = 0; i < labels.size(); i++)
        {
            QRadioButton *b = new QRadioButton(labels[i], g);
            if (i == def) b->setChecked(true);
            gl->addWidget(b);
            btns.append(b);
        }
        return QPair<QGroupBox *, QList<QRadioButton *>>(g, btns);
    };

    auto tracksSel = makeGroup(tr("Tracks"), {"40", "80"}, 0);
    auto sidesSel  = makeGroup(tr("Sides"), {tr("Single"), tr("Double")}, 0);
    auto formatSel = makeGroup(tr("Format"), {"DATA", "SYSTEM", "VENDOR"}, 0);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    layout->addWidget(tracksSel.first);
    layout->addWidget(sidesSel.first);
    layout->addWidget(formatSel.first);
    layout->addWidget(buttons);

    if (dlg.exec() != QDialog::Accepted)
        return;

    const int numTracks = tracksSel.second[0]->isChecked() ? 40 : 80;
    const int numSides  = sidesSel.second[0]->isChecked() ? 1 : 2;
    const BYTE idBase   = formatSel.second[0]->isChecked() ? 0xC1
                        : formatSel.second[1]->isChecked() ? 0x41
                        : 0x01;

    const int sectorsPerTrack = 9;
    const int sectorSize = 512;
    const int trackSize = 0x100 + sectorsPerTrack * sectorSize;

    QByteArray dsk(0x100 + numTracks * numSides * trackSize, '\0');
    char *p = dsk.data();

    memcpy(p, "MV - CPCEMU Disk-File\r\nDisk-Info\r\n", 34);
    memcpy(p + 0x22, "Cadence       ", 14);
    p[0x30] = numTracks;
    p[0x31] = numSides;
    p[0x32] = trackSize & 0xFF;
    p[0x33] = (trackSize >> 8) & 0xFF;

    for (int t = 0; t < numTracks; t++)
    {
        for (int s = 0; s < numSides; s++)
        {
            char *tp = p + 0x100 + (t * numSides + s) * trackSize;
            memcpy(tp, "Track-Info\r\n", 12);
            tp[0x10] = t;
            tp[0x11] = s;
            tp[0x14] = 2;
            tp[0x15] = sectorsPerTrack;
            tp[0x16] = 0x4E;
            tp[0x17] = 0xE5;
            for (int i = 0; i < sectorsPerTrack; i++)
            {
                char *si = tp + 0x18 + i * 8;
                si[0] = t;
                si[1] = s;
                si[2] = idBase + i;
                si[3] = 2;
            }
            memset(tp + 0x100, 0xE5, sectorsPerTrack * sectorSize);
        }
    }

    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly) || f.write(dsk) != dsk.size())
    {
        QMessageBox::warning(this, tr("New blank disc"), tr("Could not write %1").arg(fileName));
        return;
    }
    f.close();

    media->LoadDiskA(fileName);
}

void MainWindow::onMenuMediaInsertDiskB()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load disc"), QDir::homePath() + "/.cadence/DSK", tr("DSK Files (*.dsk)"));
    if (fileName != nullptr)
        media->LoadDiskB(fileName);
}

void MainWindow::onMenuMediaRemoveTape()
{
    media->EjectTape();
}

void MainWindow::onMenuMediaRemoveDiskA()
{
    media->EjectDiskA();
}

void MainWindow::onMenuMediaRemoveDiskB()
{
    media->EjectDiskB();
}

void MainWindow::onMenuROMLoadFromFile()
{
    ROMBoxDialog dlg(&settings, this);
    dlg.exec();
}

void MainWindow::applyROMOverrides()
{
    for (int i = 0; i < 16; i++)
    {
        const QString &path = settings.romPaths[i];
        if (!path.isEmpty() && QFileInfo::exists(path))
            CPC::ReadROM((char *)path.toUtf8().data(), i);
    }
}

void MainWindow::onMenuMediaRemoveCartridge()
{
    media->EjectCartridge();
}

void MainWindow::onMenuMediaInsertCartridge()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load Cartridge"), QDir::homePath() + "/.cadence/CPR", tr("Cartridge Files (*.cpr *.bin *.CPR *.BIN)"));
    if (fileName != nullptr)
        media->LoadCartridge(fileName);
}

void MainWindow::onMenuAbout()
{
    AboutDialog(this).exec();
}

void MainWindow::onMenuScreenGreenMonitor()
{
    CPC::gateArray.SetMonochrome(ui->actionGreen_monitor->isChecked());
}

void MainWindow::onMenuViewFullScreen()
{
    QSize size = QSize(768, 544);
    if (ui->actionFull_screen->isChecked())
    {
        float ratio = (float)size.height() / size.width();
        QScreen *screen = this->windowHandle()->screen();
        QSize screenSize = screen->size();
        setMinimumSize(0, 0);
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        ui->centralwidget->setFixedSize(screenSize);
        statusBar()->hide();
        this->menuBar()->setFixedHeight(0);
        ui->centralwidget->setStyleSheet("background-color:black;");
        float height = screenSize.height();
        ui->openGLWidget->setFixedSize(QSize(height / ratio, height));
        showFullScreen();
    }
    else
    {
        this->menuBar()->setFixedHeight(23);
        showNormal();
        ui->openGLWidget->setFixedSize(size);
        ui->centralwidget->setStyleSheet("background-color:gray;");
        ui->centralwidget->setFixedSize(771, 547);
        statusBar()->show();
        adjustSize();
        setFixedSize(this->size());
    }
}

void MainWindow::setMediaText(QLabel *label, const QString &text)
{
    label->setProperty("fullText", text);
    QFontMetrics fm(label->font());
    label->setText(fm.elidedText(text, Qt::ElideRight, label->width()));
}

void MainWindow::SwitchMachine(CPCType type)
{
    StopThreads();
    CPC::Finalize();
    CPC::cpcType = type;
    CPC::Init();
    applyROMOverrides();
    StartThreads();
    switch (type) {
    case CPCType::CPC464:  settings.system = "CPC464";  break;
    case CPCType::CPC664:  settings.system = "CPC664";  break;
    case CPCType::CPC6128: settings.system = "CPC6128"; break;
    }
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

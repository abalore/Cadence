#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Debugger.h"
#include "speedcontroller.h"
#include "EmulatorThread.h"
#include "CPC.h"
#include "Tape.h"
#include "FDC.h"
#include "GateArray.h"
#include "CRTScreen.h"
#include "Keyboard.h"
#include "AboutDialog.h"
#include "ROMBoxDialog.h"
#include "Settings.h"
#include "PreferencesDialog.h"
#include <QCloseEvent>
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
#include <QStyle>
#include <QVBoxLayout>
#include <QActionGroup>
#include <QMessageBox>
#include <QLabel>
#include <QStatusBar>
#include <QFileInfo>
#include <QIcon>
#include <QPainter>
#include <QImage>
#include <QFontMetrics>
#include <QTimer>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QApplication>
#include <QCursor>
#include <QGuiApplication>
#include <QScreen>

using namespace std;

MainWindow *MainWindow::Instance;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QString("%1 %2").arg(APP_NAME, APP_VERSION));
    ui->menuSettings->menuAction()->setMenuRole(QAction::NoRole);
    setAcceptDrops(true);
    QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen) screen = QApplication::primaryScreen();
    if (screen)
    {
        QRect avail = screen->availableGeometry();
        QSize hint = sizeHint();
        move(avail.center() - QPoint(hint.width() / 2, hint.height() / 2));
    }

    debugger = new Debugger(this);
    debugger->hide();
    graphicsInspector = new GraphicsInspector(this);
    assemblerWindow = nullptr;

    Instance = this;

    settings.Load();
    if (settings.system == "CPC464")       CPC::cpcType = CPCType::CPC464;
    else if (settings.system == "CPC664")  CPC::cpcType = CPCType::CPC664;
    else                                   CPC::cpcType = CPCType::CPC6128;
    CPC::has512kExpansion = settings.ram512kExpansion;
    EmulatorThread::breakpointsEnabled = settings.breakpointsEnabled;
    ui->actionEnable_breakpoints->setChecked(settings.breakpointsEnabled);
    connect(ui->actionEnable_breakpoints, &QAction::toggled, this, [](bool on){ EmulatorThread::breakpointsEnabled = on; });
    connect(ui->actionUnlock_speed, &QAction::toggled, this, [](bool on){ SpeedController::unlocked = on; });
    Keyboard::joystickEmulation = settings.joystickEmulation;
    ui->actionJoystick_emulation->setChecked(settings.joystickEmulation);
    connect(ui->actionJoystick_emulation, &QAction::toggled, this, [](bool on){ Keyboard::joystickEmulation = on; });
    for (int i = 0; i < 65536; i++) CPC::Breakpoint[i] = false;
    for (int a : settings.breakpoints) CPC::Breakpoint[a & 0xFFFF] = true;

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
    connect(ui->actionTape_rewind, &QAction::triggered, this, []{ CPC::tape.Rewind(); });
    connect(ui->actionRemove_disk, &QAction::triggered, this, &MainWindow::onMenuMediaRemoveDiskA);
    connect(ui->actionInsert_disk_2, &QAction::triggered, this, &MainWindow::onMenuMediaInsertDiskB);
    connect(ui->actionRemove_disk_2, &QAction::triggered, this, &MainWindow::onMenuMediaRemoveDiskB);
    connect(ui->actionInspect_video_memory, &QAction::triggered, this, &MainWindow::onMenuScreenInspectGraphics);
    connect(ui->actionSmooth, &QAction::changed, this, &MainWindow::onMenuScreenSmooth);
    connect(ui->actionInsert_disk, &QAction::triggered, this, &MainWindow::onMenuMediaInsertDiskA);
    connect(ui->actionNew_blank_disk, &QAction::triggered, this, &MainWindow::onMenuMediaNewBlankDiskA);
    connect(ui->actionWrite_protect_A, &QAction::changed, this, &MainWindow::onMenuMediaWriteProtectA);
    connect(ui->actionWrite_protect_B, &QAction::changed, this, &MainWindow::onMenuMediaWriteProtectB);
    connect(ui->actionROM_Box, &QAction::triggered, this, &MainWindow::onMenuROMLoadFromFile);
    connect(ui->actionRemove_cartridge, &QAction::triggered, this, &MainWindow::onMenuMediaRemoveCartridge);
    connect(ui->actionInsert_cartridge, &QAction::triggered, this, &MainWindow::onMenuMediaInsertCartridge);
    connect(ui->actionInsert_blank_cartridge, &QAction::triggered, this, &MainWindow::onMenuMediaInsertBlankCartridge);
    connect(ui->actionGreen_monitor, &QAction::changed, this, &MainWindow::onMenuScreenGreenMonitor);

    QActionGroup *persistenceGroup = new QActionGroup(this);
    persistenceGroup->setExclusive(true);
    QAction *persistenceActions[6] = {
        ui->actionPersistence_0, ui->actionPersistence_1, ui->actionPersistence_2,
        ui->actionPersistence_3, ui->actionPersistence_4, ui->actionPersistence_5
    };
    for (int i = 0; i < 6; i++)
    {
        persistenceActions[i]->setData(i);
        persistenceGroup->addAction(persistenceActions[i]);
    }
    connect(persistenceGroup, &QActionGroup::triggered, this, &MainWindow::onMenuScreenPhosphorPersistence);
    connect(ui->actionFull_screen, &QAction::changed, this, &MainWindow::onMenuViewFullScreen);
    connect(ui->actionAssembler, &QAction::triggered, this, &MainWindow::onMenuDebugAssembler);

    QActionGroup *modelGroup = new QActionGroup(this);
    modelGroup->setExclusive(true);
    modelGroup->addAction(ui->actionAmstrad_CPC464);
    modelGroup->addAction(ui->actionAmstrad_CPC664);
    modelGroup->addAction(ui->actionAmstrad_CPC6128);

    connect(ui->actionAmstrad_CPC464, &QAction::triggered, this, &MainWindow::SetCPC464);
    connect(ui->actionAmstrad_CPC664, &QAction::triggered, this, &MainWindow::SetCPC664);
    connect(ui->actionAmstrad_CPC6128, &QAction::triggered, this, &MainWindow::SetCPC6128);

    QActionGroup *crtcGroup = new QActionGroup(this);
    crtcGroup->setExclusive(true);
    QAction *crtcActions[5] = {
        ui->actionCRTC_0, ui->actionCRTC_1, ui->actionCRTC_2,
        ui->actionCRTC_3, ui->actionCRTC_4
    };
    for (int i = 0; i < 5; i++)
    {
        crtcGroup->addAction(crtcActions[i]);
        connect(crtcActions[i], &QAction::triggered, this, [i]{ CPC::crtc.crtcType = i; });
    }
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionAudio_enabled, &QAction::toggled, this, [](bool checked){ SoundThread::enabled = checked; });
    connect(ui->actionSFX_enabled, &QAction::toggled, this, [](bool checked){ SoundThread::sfxEnabled = checked; });
    connect(ui->actionTape_enabled, &QAction::toggled, this, [](bool checked){ CPC::tape.audioEnabled = checked; });
    connect(ui->actionRight_shift_as_backslash, &QAction::toggled, this, [](bool checked){ Keyboard::translation[53] = checked ? 62 : 52; });
    connect(ui->action512kExpansion, &QAction::toggled, this, &MainWindow::onToggle512kExpansion);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onMenuAbout);
    connect(ui->actionPreferences, &QAction::triggered, this, &MainWindow::onMenuPreferences);

    ui->hLine->setVisible(false);
    ui->vLine->setVisible(false);

    menuBar()->setStyleSheet(
        "QMenu::separator { background: #606060; height: 1px; margin: 4px 6px; }"
    );

    statusBar()->setSizeGripEnabled(false);
    statusBar()->setStyleSheet(
        "QStatusBar::item { border: none; }"
        "QWidget#mediaChip {"
        "  border: 1px solid #606060;"
        "  border-radius: 4px;"
        "  background-color: #202020;"
        "}"
        "QWidget#mediaChip:hover {"
        "  background-color: #6a8fbf;"
        "  border-color: #3a5a8f;"
        "}"
        "QWidget#motorChip {"
        "  border: 1px solid #606060;"
        "  border-radius: 4px;"
        "  background-color: #202020;"
        "}"
        "QWidget#mediaChip QLabel, QWidget#motorChip QLabel { color: #FFFFFF; }"
        "QWidget#mediaChip QLabel[mediaEmpty=\"true\"] { color: #606060; }"
    );

    const int iconSize = 24;
    const int outlineRadius = 1;
    const int sectionWidth = 771 / 5;
    const int textWidth = sectionWidth - iconSize - 20;
    auto outlinePixmap = [](const QPixmap &src, int radius) -> QPixmap {
        QImage mask = src.toImage().convertToFormat(QImage::Format_ARGB32);
        for (int y = 0; y < mask.height(); y++)
        {
            QRgb *row = reinterpret_cast<QRgb *>(mask.scanLine(y));
            for (int x = 0; x < mask.width(); x++)
                row[x] = qRgba(0x60, 0x60, 0x60, qAlpha(row[x]));
        }
        QPixmap maskPm = QPixmap::fromImage(mask);
        QPixmap dst(src.width() + radius * 2, src.height() + radius * 2);
        dst.fill(Qt::transparent);
        QPainter p(&dst);
        for (int dy = -radius; dy <= radius; dy++)
            for (int dx = -radius; dx <= radius; dx++)
                if ((dx || dy) && dx * dx + dy * dy <= radius * radius)
                    p.drawPixmap(radius + dx, radius + dy, maskPm);
        p.drawPixmap(radius, radius, src);
        return dst;
    };
    auto leftStretch = new QWidget(this);
    leftStretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    statusBar()->addWidget(leftStretch, 1);
    auto addMediaChip = [this, outlinePixmap](const QString &iconPath, QLabel *&textLabel, QWidget *&chipOut) {
        QWidget *chip = new QWidget(this);
        chip->setObjectName("mediaChip");
        chip->setCursor(Qt::PointingHandCursor);
        auto *h = new QHBoxLayout(chip);
        h->setContentsMargins(6, 2, 8, 2);
        h->setSpacing(6);
        QLabel *icon = new QLabel(chip);
        icon->setPixmap(outlinePixmap(QIcon(iconPath).pixmap(iconSize, iconSize), outlineRadius));
        icon->setAttribute(Qt::WA_TransparentForMouseEvents);
        textLabel = new QLabel(chip);
        textLabel->setFixedWidth(textWidth);
        textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        textLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        h->addWidget(icon);
        h->addWidget(textLabel);
        statusBar()->addWidget(chip);
        chipOut = chip;
        setMediaText(textLabel, "<Empty>");
    };
    addMediaChip(":/images/tape.png",      tapeLabel,      tapeChip);
    addMediaChip(":/images/disk.png",      diskLabel,      diskChip);
    addMediaChip(":/images/disk.png",      diskBLabel,     diskBChip);

    ledOnPixmap = outlinePixmap(QIcon(":/images/led_on.png").pixmap(iconSize, iconSize / 2), outlineRadius);
    ledOffPixmap = outlinePixmap(QIcon(":/images/led_off.png").pixmap(iconSize, iconSize / 2), outlineRadius);
    {
        QWidget *motorChip = new QWidget(this);
        motorChip->setObjectName("motorChip");
        auto *h = new QHBoxLayout(motorChip);
        h->setContentsMargins(6, 2, 8, 2);
        h->setSpacing(6);
        motorLabel = new QLabel(motorChip);
        motorLabel->setPixmap(ledOffPixmap);
        h->addWidget(motorLabel);
        statusBar()->addWidget(motorChip);
    }

    addMediaChip(":/images/cartridge.png", cartridgeLabel, cartridgeChip);
    cartridgeChip->installEventFilter(this);
    tapeChip->installEventFilter(this);
    diskChip->installEventFilter(this);
    diskBChip->installEventFilter(this);

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

void MainWindow::RefreshDebuggerIfOpen()
{
    if (debugger && debugger->isVisible())
        debugger->Update();
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    if (assemblerWindow && !assemblerWindow->close())
    {
        event->ignore();
        return;
    }
    if (media) media->SaveCartridgeIfDirty();
    event->accept();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *ev)
{
    if (ev->type() == QEvent::MouseButtonDblClick)
    {
        if (obj == tapeChip)
        {
            if (media->TapePath().isEmpty()) onMenuMediaInsertTape();
            else                             media->EjectTape();
            return true;
        }
        if (obj == cartridgeChip)
        {
            if (CPC::cartridgeEnabled) onMenuMediaRemoveCartridge();
            else                       onMenuMediaInsertCartridge();
            return true;
        }
        if (obj == diskChip)
        {
            if (media->DiskAPath().isEmpty()) onMenuMediaInsertDiskA();
            else                              media->EjectDiskA();
            return true;
        }
        if (obj == diskBChip)
        {
            if (media->DiskBPath().isEmpty()) onMenuMediaInsertDiskB();
            else                              media->EjectDiskB();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, ev);
}

void MainWindow::onMenuDebugAssembler()
{
    if (!assemblerWindow)
    {
        assemblerWindow = new AssemblerWindow(this);
        assemblerWindow->setAttribute(Qt::WA_DeleteOnClose);
        connect(assemblerWindow, &QObject::destroyed, this, [this]() { assemblerWindow = nullptr; });
        assemblerWindow->move(pos() + QPoint(100, 100));
    }
    assemblerWindow->show();
    assemblerWindow->raise();
    assemblerWindow->activateWindow();
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

    QAction *crtcActions[5] = {
        ui->actionCRTC_0, ui->actionCRTC_1, ui->actionCRTC_2,
        ui->actionCRTC_3, ui->actionCRTC_4
    };
    const int ct = (settings.crtcType >= 0 && settings.crtcType < 5) ? settings.crtcType : 0;
    crtcActions[ct]->setChecked(true);
    CPC::crtc.crtcType = ct;

    ui->action512kExpansion->setChecked(settings.ram512kExpansion);
    CPC::has512kExpansion = settings.ram512kExpansion;

    int pLevel = settings.phosphorPersistence;
    if (pLevel < 0 || pLevel > 5) pLevel = 0;
    QAction *persistenceActions[6] = {
        ui->actionPersistence_0, ui->actionPersistence_1, ui->actionPersistence_2,
        ui->actionPersistence_3, ui->actionPersistence_4, ui->actionPersistence_5
    };
    persistenceActions[pLevel]->setChecked(true);

    const bool smooth = settings.smooth;
    QTimer::singleShot(0, this, [this, smooth, pLevel]{
        ui->openGLWidget->setSmoothing(smooth);
        ui->openGLWidget->setPersistence(pLevel);
    });
    CPC::gateArray.SetMonochrome(settings.greenMonitor);
    SoundThread::enabled = settings.audioEnabled;
    SoundThread::sfxEnabled = settings.sfxEnabled;
    CPC::tape.audioEnabled = settings.tapeEnabled;
    Keyboard::translation[53] = settings.rsBackslash ? 62 : 52;

    if (settings.fullScreen)
        QTimer::singleShot(150, this, [this]{ ui->actionFull_screen->setChecked(true); });
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
    settings.crtcType     = CPC::crtc.crtcType;
    settings.ram512kExpansion = ui->action512kExpansion->isChecked();
    settings.breakpointsEnabled = EmulatorThread::breakpointsEnabled;
    settings.joystickEmulation = Keyboard::joystickEmulation;
    settings.breakpoints.clear();
    for (int i = 0; i < 65536; i++)
        if (CPC::Breakpoint[i]) settings.breakpoints.append(i);
}

void MainWindow::onMediaChanged(MediaSlot slot, const QString &text)
{
    switch (slot) {
    case MediaSlot::Tape:      tapeBaseText = text; lastTapePct = -1; setMediaText(tapeLabel, text); break;
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

void MainWindow::onEmulatorPaused()
{
    ui->hLine->setVisible(true);
    ui->vLine->setVisible(true);
    ui->hLine->move(0, CPC::screen.vPos * 2 - 56);
    ui->vLine->move(CPC::screen.hPos + 240, 0);
    ui->vLine->pos().setX(CPC::screen.vPos);
    if (!suppressNextPauseDebugger && debugger->isHidden())
        debugger->show();
    suppressNextPauseDebugger = false;
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

    if (!tapeBaseText.isEmpty() && tapeBaseText != "<Empty>")
    {
        int pct = CPC::tape.GetProgressPercent();
        if (pct != lastTapePct)
        {
            lastTapePct = pct;
            QString suffix = pct > 0 ? QString(" (%1%)").arg(pct) : QString();
            QFontMetrics fm(tapeLabel->font());
            int avail = tapeLabel->width() - fm.horizontalAdvance(suffix) - 4;
            QString name = fm.elidedText(tapeBaseText, Qt::ElideRight, qMax(0, avail));
            tapeLabel->setText(name + suffix);
        }
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
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load tape"), Settings::CadenceDir() + "/CDT", tr("Tape Files (*.cdt *.wav)"), nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName != nullptr)
        media->LoadTape(fileName);
}

void MainWindow::onMenuScreenSmooth()
{
    ui->openGLWidget->setSmoothing(ui->actionSmooth->isChecked());
}

void MainWindow::onMenuMediaInsertDiskA()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load disc"), Settings::CadenceDir() + "/DSK", tr("DSK Files (*.dsk)"), nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName != nullptr)
        media->LoadDiskA(fileName);
}

void MainWindow::onMenuMediaWriteProtectA()
{
    CPC::fdc.GetDrive(0)->SetWriteProtect(ui->actionWrite_protect_A->isChecked());
}

void MainWindow::onMenuMediaWriteProtectB()
{
    CPC::fdc.GetDrive(1)->SetWriteProtect(ui->actionWrite_protect_B->isChecked());
}

void MainWindow::onMenuMediaNewBlankDiskA()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("New blank disc"), Settings::CadenceDir() + "/DSK", tr("DSK Files (*.dsk)"), nullptr, QFileDialog::DontUseNativeDialog);
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
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load disc"), Settings::CadenceDir() + "/DSK", tr("DSK Files (*.dsk)"), nullptr, QFileDialog::DontUseNativeDialog);
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

void MainWindow::onMenuPreferences()
{
    collectSettingsFromUi();
    PreferencesDialog dlg(settings, SpeedController::unlocked, this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    applyPreferences(dlg.result(), dlg.unlockSpeed());
}

void MainWindow::applyPreferences(const Settings &ns, bool newUnlockSpeed)
{
    if (ns.smooth != settings.smooth)
        ui->actionSmooth->setChecked(ns.smooth);
    if (ns.greenMonitor != settings.greenMonitor)
        ui->actionGreen_monitor->setChecked(ns.greenMonitor);
    if (ns.phosphorPersistence != settings.phosphorPersistence)
    {
        settings.phosphorPersistence = ns.phosphorPersistence;
        QAction *pa[6] = {
            ui->actionPersistence_0, ui->actionPersistence_1, ui->actionPersistence_2,
            ui->actionPersistence_3, ui->actionPersistence_4, ui->actionPersistence_5
        };
        int level = (ns.phosphorPersistence >= 0 && ns.phosphorPersistence <= 5)
                        ? ns.phosphorPersistence : 0;
        pa[level]->setChecked(true);
        ui->openGLWidget->setPersistence(level);
    }
    if (ns.audioEnabled != settings.audioEnabled)
        ui->actionAudio_enabled->setChecked(ns.audioEnabled);
    if (ns.sfxEnabled != settings.sfxEnabled)
        ui->actionSFX_enabled->setChecked(ns.sfxEnabled);
    if (ns.tapeEnabled != settings.tapeEnabled)
        ui->actionTape_enabled->setChecked(ns.tapeEnabled);
    if (ns.rsBackslash != settings.rsBackslash)
        ui->actionRight_shift_as_backslash->setChecked(ns.rsBackslash);
    if (ns.joystickEmulation != settings.joystickEmulation)
        ui->actionJoystick_emulation->setChecked(ns.joystickEmulation);
    if (ns.crtcType != settings.crtcType)
    {
        QAction *ca[5] = {
            ui->actionCRTC_0, ui->actionCRTC_1, ui->actionCRTC_2,
            ui->actionCRTC_3, ui->actionCRTC_4
        };
        int ct = (ns.crtcType >= 0 && ns.crtcType < 5) ? ns.crtcType : 0;
        ca[ct]->setChecked(true);
        CPC::crtc.crtcType = ct;
        settings.crtcType = ct;
    }

    if (newUnlockSpeed != SpeedController::unlocked)
        ui->actionUnlock_speed->setChecked(newUnlockSpeed);

    bool romPathsChanged = false;
    for (int i = 0; i < 16; i++)
    {
        if (ns.romPaths[i] != settings.romPaths[i])
        {
            romPathsChanged = true;
            settings.romPaths[i] = ns.romPaths[i];
        }
    }
    if (romPathsChanged)
        applyROMOverrides();

    if (ns.ram512kExpansion != settings.ram512kExpansion)
        ui->action512kExpansion->setChecked(ns.ram512kExpansion);

    if (ns.fullScreen != settings.fullScreen)
        ui->actionFull_screen->setChecked(ns.fullScreen);
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
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load Cartridge"), Settings::CadenceDir() + "/CPR", tr("Cartridge Files (*.cpr *.bin *.CPR *.BIN)"), nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName != nullptr)
        media->LoadCartridge(fileName);
}

void MainWindow::onMenuMediaInsertBlankCartridge()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("New blank cartridge"),
        Settings::CadenceDir() + "/CPR",
        tr("Cartridge Files (*.cpr)"),
        nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty()) return;
    if (!fileName.endsWith(".cpr", Qt::CaseInsensitive)) fileName += ".cpr";
    QDir().mkpath(QFileInfo(fileName).absolutePath());
    if (!media->NewBlankCartridge(fileName))
        QMessageBox::warning(this, tr("New blank cartridge"),
                             tr("Could not create %1").arg(fileName));
}

void MainWindow::onMenuAbout()
{
    AboutDialog(this).exec();
}

void MainWindow::onMenuScreenGreenMonitor()
{
    CPC::gateArray.SetMonochrome(ui->actionGreen_monitor->isChecked());
}

void MainWindow::onMenuScreenPhosphorPersistence(QAction *action)
{
    int level = action->data().toInt();
    ui->openGLWidget->setPersistence(level);
    settings.phosphorPersistence = level;
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
        if (QScreen *screen = this->windowHandle() ? this->windowHandle()->screen() : QApplication::primaryScreen())
        {
            QRect avail = screen->availableGeometry();
            move(avail.center() - QPoint(width() / 2, height() / 2));
        }
    }
}

void MainWindow::setMediaText(QLabel *label, const QString &text)
{
    label->setProperty("fullText", text);
    bool empty = (text == "<Empty>" || text.isEmpty());
    label->setProperty("mediaEmpty", empty);
    label->style()->unpolish(label);
    label->style()->polish(label);
    QFontMetrics fm(label->font());
    label->setText(fm.elidedText(text, Qt::ElideRight, label->width()));
}

void MainWindow::onToggle512kExpansion(bool checked)
{
    if (CPC::has512kExpansion == checked) return;
    StopThreads();
    CPC::Finalize();
    CPC::has512kExpansion = checked;
    CPC::Init();
    applyROMOverrides();
    StartThreads();
    settings.ram512kExpansion = checked;
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

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (!event->mimeData()->hasUrls())
        return;
    for (const QUrl &url : event->mimeData()->urls())
    {
        if (!url.isLocalFile())
            continue;
        const QString ext = QFileInfo(url.toLocalFile()).suffix().toLower();
        if (ext == "dsk" || ext == "cdt" || ext == "wav" || ext == "cpr")
        {
            event->acceptProposedAction();
            return;
        }
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasUrls())
        return;
    for (const QUrl &url : event->mimeData()->urls())
    {
        if (!url.isLocalFile())
            continue;
        const QString path = url.toLocalFile();
        const QString ext = QFileInfo(path).suffix().toLower();
        if (ext == "dsk")
            media->LoadDiskA(path);
        else if (ext == "cdt" || ext == "wav")
            media->LoadTape(path);
        else if (ext == "cpr")
            media->LoadCartridge(path);
    }
    event->acceptProposedAction();
}

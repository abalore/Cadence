#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "EmulatorThread.h"
#include "SoundThread.h"
#include "Debugger.h"
#include "graphicsinspector.h"
#include "enterbytesdialog.h"
#include "MediaController.h"
#include "Settings.h"
#include "CPC.h"
#include "AssemblerWindow.h"
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QTimerEvent>
#include <QLabel>
#include <QPixmap>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    static MainWindow *Instance;
    void RefreshDebuggerIfOpen();
    void ResetEmulation();
protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *ev) override;
private slots:
    void onEmulatorPaused();
    void onEmulatorResumed();
    void onEmulatorFinishedFrame();
    void onMenuMemoryLoadBinaryFile();
    void onMenuMemorySaveBinaryFile();
    void onMenuMediaInsertTape();
    void onMenuMediaRemoveTape();
    void onMenuMediaRemoveDiskA();
    void onMenuMediaInsertDiskB();
    void onMenuMediaRemoveDiskB();
    void onMenuScreenSmooth();
    void onMenuScreenInspectGraphics();
    void onMenuMediaInsertDiskA();
    void onMenuMediaNewBlankDiskA();
    void onMenuMediaWriteProtectA();
    void onMenuMediaWriteProtectB();
    void onMenuROMLoadFromFile();
    void onMenuMemoryEnterBytes();
    void onMenuMediaRemoveCartridge();
    void onMenuMediaInsertCartridge();
    void onMenuMediaInsertBlankCartridge();
    void onMenuScreenGreenMonitor();
    void onMenuViewFullScreen();
    void onMenuDebugAssembler();
    void onMenuAbout();
    void onMediaChanged(MediaSlot slot, const QString &text);
    void StartThreads();
    void StopThreads();

    void SetCPC464();
    void SetCPC664();
    void SetCPC6128();
private:
    void SwitchMachine(CPCType type);
    void setMediaText(QLabel *label, const QString &text);
    void applySettingsToUi();
    void applyROMOverrides();
    void collectSettingsFromUi();
    Ui::MainWindow *ui;
    Debugger *debugger;
    GraphicsInspector *graphicsInspector;
    EnterBytesDialog *enterBytesDialog;
    AssemblerWindow *assemblerWindow;
    EmulatorThread *workerThread;
    SoundThread *soundThread;
    MediaController *media;
    Settings settings;
    QLabel *motorLabel;
    QPixmap ledOnPixmap;
    QPixmap ledOffPixmap;
    QLabel *diskLabel;
    QLabel *diskBLabel;
    QLabel *tapeLabel;
    QLabel *cartridgeLabel;
    QWidget *diskChip;
    QWidget *diskBChip;
    QWidget *tapeChip;
    QWidget *cartridgeChip;
    bool aboutShown = false;
};
#endif // MAINWINDOW_H

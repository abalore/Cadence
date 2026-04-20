#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "EmulatorThread.h"
#include "SoundThread.h"
#include "Debugger.h"
#include "graphicsinspector.h"
#include "enterbytesdialog.h"
#include "CPC.h"
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
    void onMenuROMLoadFromFile();
    void onMenuMemoryEnterBytes();
    void onMenuMediaRemoveCartridge();
    void onMenuMediaInsertCartridge();
    void onMenuScreenGreenMonitor();
    void onMenuViewFullScreen();
    void StartThreads();
    void StopThreads();
    void ResetEmulation();

    void SetCPC464();
    void SetCPC664();
    void SetCPC6128();
private:
    void SwitchMachine(CPCType type);
    void setMediaText(QLabel *label, const QString &text);
    void loadSettings();
    void saveSettings();
    Ui::MainWindow *ui;
    Debugger *debugger;
    GraphicsInspector *graphicsInspector;
    EnterBytesDialog *enterBytesDialog;
    EmulatorThread *workerThread;
    SoundThread *soundThread;
    QLabel *motorLabel;
    QPixmap ledOnPixmap;
    QPixmap ledOffPixmap;
    QLabel *diskLabel;
    QLabel *diskBLabel;
    QLabel *tapeLabel;
    QLabel *cartridgeLabel;
    QString diskAPath;
    QString diskBPath;
    QString tapePath;
    QString cartridgePath;
    bool loadDiskA(const QString &path);
    bool loadDiskB(const QString &path);
    bool loadTape(const QString &path);
    bool loadCartridge(const QString &path);
};
#endif // MAINWINDOW_H

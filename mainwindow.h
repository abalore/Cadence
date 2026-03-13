#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "EmulatorThread.h"
#include "SoundThread.h"
#include "Debugger.h"
#include "graphicsinspector.h"
#include "enterbytesdialog.h"
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QTimerEvent>

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
    void onEmulatorFinishedFrame();
    void onMenuMemoryLoadBinaryFile();
    void onMenuMemorySaveBinaryFile();
    void onMenuMediaInsertTape();
    void onMenuScreenSmooth();
    void onMenuScreenInspectGraphics();
    void onMenuMediaInsertDiskA();
    void onMenuROMLoadFromFile();
    void onMenuMemoryEnterBytes();
    void onMenuMediaRemoveCartridge();
    void onMenuMediaInsertCartridge();
    void onMenuScreenGreenMonitor();
    void StartThreads();
    void StopThreads();
    void ResetEmulation();

    void SetCPC464();
    void SetCPC664();
    void SetCPC6128();
private:
    Ui::MainWindow *ui;
    Debugger *debugger;
    GraphicsInspector *graphicsInspector;
    EnterBytesDialog *enterBytesDialog;
    EmulatorThread *workerThread;
    SoundThread *soundThread;
};
#endif // MAINWINDOW_H

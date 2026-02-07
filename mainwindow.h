#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "EmulatorWorkerThread.h"
#include "SoundThread.h"
#include "Debugger.h"
#include "graphicsinspector.h"
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
    void timerEvent(QTimerEvent *event) override;
    void onMenuFileLoadBinary();
    void onMenuDebugPause();
    void onMenuDebugReset();
    void onEmulatorPaused();
    void onEmulatorFinishedFrame();
    void onMenuTapeLoadWAV();
    void onMenuTapeLoadCDT();
    void onMenuDebugInspectGraphics();
private:
    Ui::MainWindow *ui;
    Debugger *debugger;
    GraphicsInspector *graphicsInspector;
    EmulatorWorkerThread *workerThread;
    SoundThread *soundThread;
};
#endif // MAINWINDOW_H

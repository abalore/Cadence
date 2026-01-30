#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "EmulatorWorkerThread.h"
#include "SoundThread.h"
#include "Debugger.h"
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

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
    void onMenuFileLoadBinary();
    void onMenuDebugPause();
    void onMenuDebugReset();
    void onEmulatorPaused();
    void onEmulatorFinishedFrame();
private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    QGraphicsPixmapItem *pixItem;
    Debugger *debugger;
    EmulatorWorkerThread *workerThread;
    SoundThread *soundThread;
};
#endif // MAINWINDOW_H

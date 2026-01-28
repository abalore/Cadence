#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "EmulatorWorkerThread.h"
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
protected:
    void timerEvent(QTimerEvent *event) override;
private slots:
    void onEmulatorPaused();
    void onMenuDebugPause();
    void onMenuDebugReset();
private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    QGraphicsPixmapItem *pixItem;
    Debugger *debugger;
    EmulatorWorkerThread *workerThread;
};
#endif // MAINWINDOW_H

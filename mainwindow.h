#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
    void on_pushButton_clicked();
    void onEmulatorPaused();
    void onStepOverClicked();
    void onStepOutClicked();
    void onRenderClicked();
private:
    void DisableDebugButtons();
    void EnableDebugButtons();
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    QGraphicsPixmapItem *pixItem;
};
#endif // MAINWINDOW_H

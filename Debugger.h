#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <QDialog>
#include <QStringListModel>
#include <QEvent>

QT_BEGIN_NAMESPACE
namespace Ui {
class Debugger;
}
QT_END_NAMESPACE

class Debugger : public QDialog
{
    Q_OBJECT
public:
    Debugger(QWidget *parent = nullptr);
    ~Debugger();
    void Update();
private slots:
    void onRunClicked();
    void onStepInClicked();
    void onStepOverClicked();
    void onStepOutClicked();
private:
    Ui::Debugger *ui;
    QStringList listDisassembly;
    QStringListModel *modelDisassembly;
    QModelIndex modelDisassemblyIndex;
    QStringList listMemory;
    QStringListModel *modelMemory;
    QModelIndex modelMemoryIndex;
    void closeEvent(QCloseEvent *event);
};

#endif // DEBUGGER_H

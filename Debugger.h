#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <QDialog>
#include <QStringListModel>
#include <QEvent>
#include <string.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class Debugger;
}
QT_END_NAMESPACE

using namespace std;

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
    void onRunToClicked();
    void onResetNopsClicked();
    void onToggleBreakpointClicked();
private:

    void UpdateZ80Panel();
    void UpdateCRTCPanel();
    void UpdateGateArrayPanel();
    string GetZ80StackDebugLine();

    Ui::Debugger *ui;
    QStringList listDisassembly;
    QStringListModel *modelDisassembly;
    QModelIndex modelDisassemblyIndex;
    QStringList listMemory;
    QStringListModel *modelMemory;
    QModelIndex modelMemoryIndex;
    uchar nextInstructionLength;
    uchar nextInstructionOpCode;
    void closeEvent(QCloseEvent *event);
};

#endif // DEBUGGER_H

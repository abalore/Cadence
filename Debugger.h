#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <QDialog>
#include <QStringListModel>
#include <QEvent>
#include <QVector>
#include <QSet>
#include <set>
#include <string.h>
#include "defs.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Debugger;
}
QT_END_NAMESPACE

using namespace std;

class DisassemblyModel : public QStringListModel
{
public:
    int pcRow = -1;
    QSet<int> bpRows;
    QVariant data(const QModelIndex &index, int role) const override;
};

class Debugger : public QDialog
{
    Q_OBJECT
public:
    Debugger(QWidget *parent = nullptr);
    ~Debugger();
    void Update();
protected:
    bool eventFilter(QObject *o, QEvent *e) override;
    void reject() override;
private slots:
    void onRunClicked();
    void onStepInClicked();
    void onStepOverClicked();
    void onStepOutClicked();
    void onRunToClicked();
    void onResetNopsClicked();
    void onToggleBreakpointClicked();
    void onGoToClicked();
    void onMemorySourceChanged(int index);
    void onMemoryDetailChanged(int index);
    void onZ80FieldEdited();
    void onCRTCFieldEdited();
    void onStackClicked(const QModelIndex &index);
private:
    enum MemSource { CpuView, RamCurrent, RamBank, LowerRom, UpperRomSlot, Cartridge };
    void PopulateMemorySources();
    void PopulateMemoryDetail();

    void UpdateZ80Panel();
    void UpdateCRTCPanel();
    void UpdateGateArrayPanel();
    void UpdateStackPanel();

    Ui::Debugger *ui;
    QStringList listDisassembly;
    QVector<word> disassemblyAddresses;
    std::set<word> manualAnchors;
    DisassemblyModel *modelDisassembly;
    QStringListModel *modelStack;
    QModelIndex modelDisassemblyIndex;
    QStringList listMemory;
    QStringListModel *modelMemory;
    QModelIndex modelMemoryIndex;
    MemSource memSource = CpuView;
    int memDetail = 0;
    uchar nextInstructionLength;
    uchar nextInstructionOpCode;
    void closeEvent(QCloseEvent *event);
};

#endif // DEBUGGER_H

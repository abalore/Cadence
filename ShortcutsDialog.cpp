#include "ShortcutsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDialogButtonBox>

namespace {
struct Entry { const char *keys; const char *desc; };
struct Section { const char *title; const Entry *entries; int count; };

const Entry kMain[] = {
    {"F1",         "Insert tape..."},
    {"Shift+F1",   "Rewind tape"},
    {"Ctrl+F1",    "Remove tape"},
    {"F2",         "Insert disc into Drive A..."},
    {"Ctrl+F2",    "Remove disc from Drive A"},
    {"F3",         "Insert cartridge..."},
    {"Ctrl+F3",    "Remove cartridge"},
    {"F5",         "Show Debugger"},
    {"F6",         "Show Assembler"},
    {"F9",         "Toggle Unlock speed"},
    {"F10",        "Toggle Joystick emulation"},
    {"F11",        "Toggle Full screen"},
    {"Ctrl+F11",   "Toggle Smooth scaling"},
    {"Shift+F11",  "Toggle Green monitor"},
    {"F12",        "Reset emulator"},
    {"Ctrl+0..5",  "Phosphor persistence (Off..5 frames)"},
    {"Alt+F4",     "Quit"},
};

const Entry kDebugger[] = {
    {"F3",         "Go to address..."},
    {"F4",         "Run to cursor"},
    {"F5",         "Run"},
    {"F6",         "Step out"},
    {"F7",         "Step in"},
    {"F8",         "Step over"},
    {"F9",         "Toggle breakpoint at cursor"},
    {"F12",        "Reset NOPS counter"},
    {"Ctrl+F",     "Find bytes (in MEM view)"},
    {"Ctrl+G",     "Find next"},
    {"Esc",        "Cancel current edit"},
};

const Entry kAssembler[] = {
    {"Ctrl+N",     "New file"},
    {"Ctrl+O",     "Open file..."},
    {"Ctrl+S",     "Save"},
    {"Ctrl+Shift+S","Save as..."},
    {"Ctrl+W",     "Close tab"},
    {"Ctrl+Z",     "Undo"},
    {"Ctrl+Shift+Z","Redo"},
    {"Ctrl+X",     "Cut"},
    {"Ctrl+C",     "Copy"},
    {"Ctrl+V",     "Paste"},
    {"Ctrl+A",     "Select all"},
};

const Section kSections[] = {
    {"Main window", kMain,      sizeof(kMain)      / sizeof(Entry)},
    {"Debugger",    kDebugger,  sizeof(kDebugger)  / sizeof(Entry)},
    {"Assembler",   kAssembler, sizeof(kAssembler) / sizeof(Entry)},
};

QGroupBox *buildSection(const Section &s, QWidget *parent)
{
    auto *group = new QGroupBox(QObject::tr(s.title), parent);
    auto *grid = new QGridLayout(group);
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(2);
    grid->setColumnStretch(0, 0);
    grid->setColumnStretch(1, 1);
    for (int i = 0; i < s.count; i++)
    {
        auto *k = new QLabel(QString("<b>%1</b>").arg(s.entries[i].keys), group);
        auto *d = new QLabel(QObject::tr(s.entries[i].desc), group);
        k->setTextFormat(Qt::RichText);
        grid->addWidget(k, i, 0, Qt::AlignTop | Qt::AlignLeft);
        grid->addWidget(d, i, 1, Qt::AlignTop | Qt::AlignLeft);
    }
    return group;
}
}

ShortcutsDialog::ShortcutsDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Keyboard shortcuts"));
    auto *main = new QVBoxLayout(this);
    auto *cols = new QHBoxLayout;
    cols->setSpacing(12);
    for (const Section &s : kSections)
        cols->addWidget(buildSection(s, this), 0, Qt::AlignTop);
    main->addLayout(cols);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    main->addWidget(buttons);
}

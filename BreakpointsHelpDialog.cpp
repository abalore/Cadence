#include "BreakpointsHelpDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <QFrame>

namespace {
struct Row { const char *a; const char *b; bool wrap; };

const Row kIcons[] = {
    {"●", "Unconditional — breaks on every visit.", false},
    {"◆", "Conditional — breaks only when the expression is true.", false},
};

const Row kKeys[] = {
    {"F9",       "Toggle breakpoint at cursor (clears any condition).", false},
    {"Shift+F9", "Edit condition at cursor (empty input = unconditional).", false},
};

const Row kOperands[] = {
    {"8-bit registers",  "A B C D E F H L IXH IXL IYH IYL R I", false},
    {"16-bit registers", "AF BC DE HL IX IY PC SP", false},
    {"Shadow registers", "AF' BC' DE' HL'", false},
    {"Flags (0/1)",      "FZ FC FS FN FH FP", false},
    {"Memory byte read", "[addr]   — e.g. [&4000]", false},
    {"Number bases",     "decimal: 123 · hex: &FF, $FF, #FF, 0xFF, 12H · binary: &X1010", false},
    {"Grouping",         "( ... )", false},
};

const Row kOperators[] = {
    {"Unary",          "-x   ~x   !x", false},
    {"Multiplicative", "*   /   %", false},
    {"Additive",       "+   -", false},
    {"Shift",          "<<   >>", false},
    {"Relational",     "<   <=   >   >=", false},
    {"Equality",       "==   !=", false},
    {"Bitwise",        "&   ^   |", false},
    {"Logical",        "&&   ||", false},
};

const Row kExamples[] = {
    {"A == &7F",                  "Break when A equals 0x7F.", false},
    {"HL >= &C000 && HL < &D000", "Break when HL is in [&C000, &D000).", false},
    {"[&4000] != 0",              "Break when the byte at &4000 is non-zero.", false},
    {"FZ",                        "Break when the Zero flag is set.", false},
    {"BC > 100 && (A & &80)",     "Break when BC > 100 AND bit 7 of A is set.", false},
    {"PC == &1234 && B != 0",     "Break at &1234 only if B is non-zero.", false},
};

QGroupBox *buildGrid(const char *title, const Row *rows, int n, QWidget *parent)
{
    auto *group = new QGroupBox(QObject::tr(title), parent);
    auto *grid  = new QGridLayout(group);
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(2);
    grid->setColumnStretch(0, 0);
    grid->setColumnStretch(1, 1);
    for (int i = 0; i < n; i++)
    {
        auto *a = new QLabel(QString("<b>%1</b>").arg(rows[i].a), group);
        auto *b = new QLabel(QObject::tr(rows[i].b), group);
        a->setTextFormat(Qt::RichText);
        b->setTextInteractionFlags(Qt::TextSelectableByMouse);
        if (rows[i].wrap)
        {
            b->setWordWrap(true);
            b->setMinimumWidth(360);
        }
        grid->addWidget(a, i, 0, Qt::AlignTop | Qt::AlignLeft);
        grid->addWidget(b, i, 1, Qt::AlignTop | Qt::AlignLeft);
    }
    return group;
}
} // namespace

BreakpointsHelpDialog::BreakpointsHelpDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Breakpoints"));

    auto *outer = new QVBoxLayout(this);

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *content = new QWidget(scroll);
    auto *main = new QVBoxLayout(content);

    main->addWidget(buildGrid("Icons in the disassembly view",
                              kIcons,     sizeof(kIcons)     / sizeof(Row), content));
    main->addWidget(buildGrid("Keyboard",
                              kKeys,      sizeof(kKeys)      / sizeof(Row), content));
    main->addWidget(buildGrid("Operands",
                              kOperands,  sizeof(kOperands)  / sizeof(Row), content));
    main->addWidget(buildGrid("Operators (high to low precedence)",
                              kOperators, sizeof(kOperators) / sizeof(Row), content));
    main->addWidget(buildGrid("Examples",
                              kExamples,  sizeof(kExamples)  / sizeof(Row), content));

    auto *note = new QLabel(
        tr("Conditions are evaluated only when execution reaches the address, so they "
           "have no impact on emulation speed when not hit. Identifiers are "
           "case-insensitive."), content);
    note->setWordWrap(true);
    main->addWidget(note);
    main->addStretch(1);

    scroll->setWidget(content);
    outer->addWidget(scroll);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    outer->addWidget(buttons);

    resize(720, 720);
}

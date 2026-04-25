#include "Debugger.h"
#include "ui_Debugger.h"
#include "EmulatorThread.h"
#include "Disassembler.h"
#include "Z80.h"
#include "PPI.h"
#include "CPC.h"
#include "CRTScreen.h"
#include "GateArray.h"
#include <QCloseEvent>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QStringListModel>
#include <QModelIndex>
#include <QScrollBar>
#include <QShortcut>
#include <QInputDialog>
#include <QBrush>
#include <QColor>
#include <QMenuBar>
#include <QMenu>
#include <algorithm>

QVariant DisassemblyModel::data(const QModelIndex &index, int role) const
{
    if (index.row() == pcRow)
    {
        if (role == Qt::BackgroundRole) return QBrush(QColor(255, 240, 180));
        if (role == Qt::ForegroundRole) return QBrush(Qt::black);
    }
    else if (bpRows.contains(index.row()))
    {
        if (role == Qt::ForegroundRole) return QBrush(QColor(220, 50, 50));
    }
    return QStringListModel::data(index, role);
}

Debugger::Debugger(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Debugger)
{
    ui->setupUi(this);
    modelDisassembly = new DisassemblyModel();
    ui->listDisassembly->setModel(modelDisassembly);
    modelMemory = new QStringListModel();
    ui->listMemory->setModel(modelMemory);
    modelStack = new QStringListModel();
    ui->listStack->setModel(modelStack);
    connect(ui->listStack, &QListView::clicked, this, &Debugger::onStackClicked);

    auto refreshKeepScroll = [this]{
        QModelIndex topIdx = ui->listDisassembly->indexAt(QPoint(0, 0));
        word topAddr = (topIdx.isValid() && topIdx.row() < disassemblyAddresses.size())
                       ? disassemblyAddresses[topIdx.row()] : 0;
        Update();
        int newRow = disassemblyAddresses.indexOf(topAddr);
        if (newRow >= 0)
            ui->listDisassembly->scrollTo(modelDisassembly->index(newRow),
                                          QAbstractItemView::PositionAtTop);
    };
    auto onDisViewChanged = [this, refreshKeepScroll]{
        bool custom = ui->rbDisCustom->isChecked();
        ui->chkDisLoRom->setEnabled(custom);
        ui->chkDisHiRom->setEnabled(custom);
        refreshKeepScroll();
    };
    connect(ui->rbDisCpu,    &QRadioButton::toggled, this, onDisViewChanged);
    connect(ui->rbDisCustom, &QRadioButton::toggled, this, onDisViewChanged);
    connect(ui->chkDisLoRom, &QCheckBox::toggled,    this, refreshKeepScroll);
    connect(ui->chkDisHiRom, &QCheckBox::toggled,    this, refreshKeepScroll);
    PopulateMemorySources();
    PopulateMemoryDetail();
    connect(ui->cmbMemorySource, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &Debugger::onMemorySourceChanged);
    connect(ui->cmbMemoryDetail, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &Debugger::onMemoryDetailChanged);
    QLineEdit *z80Fields[] = {
        ui->txtAF, ui->txtBC, ui->txtDE, ui->txtHL,
        ui->txtAF_, ui->txtBC_, ui->txtDE_, ui->txtHL_,
        ui->txtIX, ui->txtIY, ui->txtPC, ui->txtSP,
        ui->txtI, ui->txtR,
        ui->txtIM, ui->txtIRQ, ui->txtIFF1, ui->txtIFF2
    };
    for (QLineEdit *f : z80Fields)
        connect(f, &QLineEdit::editingFinished, this, &Debugger::onZ80FieldEdited);

    QLineEdit *crtcFields[] = {
        ui->txtHT, ui->txtHCC, ui->txtHD, ui->txtHSP, ui->txtHSW, ui->txtHSC,
        ui->txtVSW, ui->txtVSC, ui->txtVT, ui->txtVCC, ui->txtVD, ui->txtVSP,
        ui->txtMRA, ui->txtRA, ui->txtVTA, ui->txtVTAC,
        ui->txtDSA, ui->txtMA
    };
    for (QLineEdit *f : crtcFields)
        connect(f, &QLineEdit::editingFinished, this, &Debugger::onCRTCFieldEdited);

    QWidget *gaWidgets[] = {
        ui->txtPen, ui->txtBorder, ui->txtMode, ui->txtVideoAddr,
        ui->txtR52, ui->txtPPI,
        ui->chkLoR, ui->chkHiR, ui->chkCartridge
    };
    for (QWidget *w : gaWidgets)
        w->setEnabled(false);

    QMenuBar *bar = new QMenuBar(this);
    bar->setGeometry(0, 0, width(), 24);
    QMenu *debugMenu = bar->addMenu("&Debug");
    debugMenu->addAction("Run",               QKeySequence(Qt::Key_F5), this, &Debugger::onRunClicked);
    debugMenu->addAction("Step In",           QKeySequence(Qt::Key_F7), this, &Debugger::onStepInClicked);
    debugMenu->addAction("Step Over",         QKeySequence(Qt::Key_F8), this, &Debugger::onStepOverClicked);
    debugMenu->addAction("Step Out",          QKeySequence(Qt::Key_F6), this, &Debugger::onStepOutClicked);
    debugMenu->addAction("Run To",            QKeySequence(Qt::Key_F4), this, &Debugger::onRunToClicked);
    debugMenu->addAction("Toggle Breakpoint", QKeySequence(Qt::Key_F9), this, &Debugger::onToggleBreakpointClicked);
    debugMenu->addAction("Go To...",          QKeySequence(Qt::Key_F3),  this, &Debugger::onGoToClicked);
    debugMenu->addAction("Reset NOPS",        QKeySequence(Qt::Key_F12), this, &Debugger::onResetNopsClicked);

    for (QObject *child : findChildren<QObject*>())
        child->installEventFilter(this);
}

Debugger::~Debugger()
{
    delete modelMemory;
    delete modelStack;
    delete modelDisassembly;
    delete ui;
}

void Debugger::closeEvent(QCloseEvent *event)
{
    EmulatorThread::Run();
    setHidden(true);
    event->ignore();
}

void Debugger::reject()
{
    if (auto *le = qobject_cast<QLineEdit*>(focusWidget()))
    {
        UpdateZ80Panel();
        le->clearFocus();
    }
}

bool Debugger::eventFilter(QObject *o, QEvent *e)
{
    QEvent::Type t = e->type();
    if (t == QEvent::Shortcut
        || (t == QEvent::MouseButtonPress && o != focusWidget()))
        if (QWidget *fw = focusWidget()) fw->clearFocus();
    return QDialog::eventFilter(o, e);
}

void Debugger::Update()
{
    int prevRow = ui->listDisassembly->currentIndex().row();
    bool hadCursor = (prevRow >= 0 && prevRow < disassemblyAddresses.size());
    word prevAddr = hadCursor ? disassemblyAddresses[prevRow] : 0;

    int pcPosition = 0;
    int instrCount = 0;
    listDisassembly.clear();
    disassemblyAddresses.clear();
    modelDisassembly->bpRows.clear();

    // Build the sorted anchor list: manual anchors + current PC + breakpoints.
    std::vector<int> anchors(manualAnchors.begin(), manualAnchors.end());
    anchors.push_back(CPC::z80.GetPC());
    for (int i = 0; i < 65536; i++)
        if (CPC::Breakpoint[i]) anchors.push_back(i);
    anchors.push_back(0x10000);
    std::sort(anchors.begin(), anchors.end());
    anchors.erase(std::unique(anchors.begin(), anchors.end()), anchors.end());

    BYTE *origPage0 = CPC::memPage[0];
    BYTE *origPage3 = CPC::memPage[3];
    if (ui->rbDisCustom->isChecked())
    {
        CPC::memPage[0] = ui->chkDisLoRom->isChecked() && CPC::LoROM ? CPC::LoROM : CPC::RAM[0];
        CPC::memPage[3] = ui->chkDisHiRom->isChecked() && CPC::HiROM ? CPC::HiROM : CPC::RAM[3];
    }

    char buff[200];
    bool pcFound = false;
    Disassembler::SetPoint(0x0000);
    while (Disassembler::addr < 0x10000)
    {
        BYTE opCode;
        BYTE instrLength;
        string label, address, bytes, instruction;
        ushort position = Disassembler::addr;
        auto it = std::upper_bound(anchors.begin(), anchors.end(), (int)position);
        int boundary = *it;
        Disassembler::GetNextInstruction(instrLength, opCode, &label, &address, &bytes, &instruction, boundary);
        sprintf(buff, "%14.14s %s %4s  %12s  %s", label.data(), CPC::Breakpoint[position] ? "● " : "  ", address.data(), bytes.data(), instruction.data());
        listDisassembly.append(buff);
        disassemblyAddresses.append(position);
        if (CPC::Breakpoint[position]) modelDisassembly->bpRows.insert(instrCount);
        if (!pcFound && position >= CPC::z80.GetPC())
        {
            pcPosition = instrCount;
            pcFound = true;
            nextInstructionLength = instrLength;
            nextInstructionOpCode = opCode;
        }
        instrCount++;
    }
    CPC::memPage[0] = origPage0;
    CPC::memPage[3] = origPage3;

    modelDisassembly -> setStringList(listDisassembly);
    modelDisassembly->pcRow = pcPosition;

    int cursorRow = hadCursor ? disassemblyAddresses.indexOf(prevAddr) : pcPosition;
    if (cursorRow < 0) cursorRow = pcPosition;
    modelDisassemblyIndex = modelDisassembly->index(cursorRow);
    ui->listDisassembly->setCurrentIndex(modelDisassemblyIndex);

    QModelIndex pcIndex = modelDisassembly->index(pcPosition);
    ui->listDisassembly->scrollTo(pcIndex, QAbstractItemView::PositionAtCenter);

    int memoryIndex = ui->listMemory->currentIndex().row();
    listMemory.clear();

    int startAddr = 0x0000;
    int endAddr   = 0xFFF0;
    int addrDigits = 4;
    BYTE *base = nullptr;
    int baseAddr = 0;
    bool unavailable = false;
    const char *unavailableMsg = nullptr;
    switch (memSource)
    {
    case CpuView:
    case RamCurrent:
        break;
    case RamBank:
        if (memDetail < 0 || memDetail >= 8 || !CPC::RAMs[memDetail]) { unavailable = true; unavailableMsg = "(RAM bank not allocated)"; }
        else { base = CPC::RAMs[memDetail]; startAddr = 0x0000; endAddr = 0x3FF0; }
        break;
    case LowerRom:
        if (!CPC::LoROM) { unavailable = true; unavailableMsg = "(Lower ROM not loaded)"; }
        else { base = CPC::LoROM; startAddr = 0x0000; endAddr = 0x3FF0; }
        break;
    case UpperRomSlot:
        if (memDetail < 0 || memDetail >= ROM_SLOTS || !CPC::HiROMs[memDetail]) { unavailable = true; unavailableMsg = "(Upper ROM slot empty)"; }
        else { base = CPC::HiROMs[memDetail]; startAddr = 0xC000; endAddr = 0xFFF0; baseAddr = 0xC000; }
        break;
    case Cartridge:
        if (!CPC::Cartridge) { unavailable = true; unavailableMsg = "(no cartridge inserted)"; }
        else if (memDetail < 0 || memDetail >= 32) { unavailable = true; unavailableMsg = "(invalid cartridge bank)"; }
        else { base = CPC::Cartridge + memDetail * 0x4000; startAddr = 0xC000; endAddr = 0xFFF0; baseAddr = 0xC000; }
        break;
    }

    ui->label->setText(addrDigits == 5
        ? "MEM    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F"
        : "MEM   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");

    if (unavailable)
    {
        listMemory.append(unavailableMsg);
    }
    else
    {
        const char *fmt = (addrDigits == 5)
            ? "%05X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X"
            : "%04X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X";
        for (int i = startAddr; i <= endAddr; i += 16)
        {
            BYTE b[16];
            for (int k = 0; k < 16; k++)
            {
                int a = i + k;
                switch (memSource)
                {
                case CpuView:      b[k] = CPC::memPage[(a >> 14) & 3][a & 0x3FFF]; break;
                case RamCurrent:   b[k] = CPC::RAM[(a >> 14) & 3][a & 0x3FFF];     break;
                default:           b[k] = base[a - baseAddr];                      break;
                }
            }
            sprintf(buff, fmt, i,
                    b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7],
                    b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15]);
            listMemory.append(buff);
        }
    }

    modelMemory -> setStringList(listMemory);
    modelMemoryIndex = modelMemory->index(memoryIndex);
    ui->listMemory->scrollTo(modelMemoryIndex, QAbstractItemView::PositionAtCenter);
    ui->listMemory->setCurrentIndex(modelMemoryIndex);

    UpdateZ80Panel();
    UpdateCRTCPanel();
    UpdateGateArrayPanel();
    UpdateStackPanel();
    ui->lblDisHiRomSel->setText(QString("Selected: #%1").arg(QString("%1").arg(CPC::selectedROM, 2, 16, QChar('0')).toUpper()));

    setEnabled(true);
}

void Debugger::onRunClicked()
{
    hide();
    EmulatorThread::Run();
}

void Debugger::onStepInClicked()
{
    EmulatorThread::RunStep();
}

void Debugger::onStepOverClicked()
{
    setEnabled(false);
    if (nextInstructionOpCode == 0x18
        || nextInstructionOpCode == 0xC3
        || nextInstructionOpCode == 0xC9
        || nextInstructionOpCode == 0xE9)
        EmulatorThread::RunStep();
    else
        EmulatorThread::RunTo(CPC::z80.GetPC() + nextInstructionLength);
}

void Debugger::onStepOutClicked()
{
    setEnabled(false);
    word address = CPC::z80.GetSP();
    BYTE L = CPC::GetByteAt(address);
    BYTE H = CPC::GetByteAt(address + 1);
    EmulatorThread::RunTo(L + H * 256);
}

void Debugger::onResetNopsClicked()
{
    CPC::z80.ResetNopCounter();
    Update();
}

void Debugger::onRunToClicked()
{
    setEnabled(false);
    int index = ui->listDisassembly->currentIndex().row();
    EmulatorThread::RunTo(disassemblyAddresses.at(index));
}

void Debugger::onToggleBreakpointClicked()
{
    int index = ui->listDisassembly->currentIndex().row();
    if (index < 0) return;
    word address = disassemblyAddresses.at(index);
    CPC::Breakpoint[address] = !CPC::Breakpoint[address];
    Update();
    QModelIndex restored = modelDisassembly->index(index);
    ui->listDisassembly->setCurrentIndex(restored);
    ui->listDisassembly->scrollTo(restored, QAbstractItemView::PositionAtCenter);
}

void Debugger::onGoToClicked()
{
    bool ok = false;
    QString input = QInputDialog::getText(this, "Go to address",
                                          "Address (hex):",
                                          QLineEdit::Normal, QString(), &ok);
    if (!ok) return;
    QString trimmed = input.trimmed();
    if (trimmed.isEmpty()) return;
    unsigned target = trimmed.toUInt(&ok, 16);
    if (!ok || target > 0xFFFF) return;

    manualAnchors.insert((word)target);
    Update();

    for (int i = 0; i < disassemblyAddresses.size(); i++)
    {
        if (disassemblyAddresses.at(i) == (word)target)
        {
            QModelIndex mi = modelDisassembly->index(i);
            ui->listDisassembly->setCurrentIndex(mi);
            ui->listDisassembly->scrollTo(mi, QAbstractItemView::PositionAtCenter);
            break;
        }
    }
}


void Debugger::UpdateZ80Panel()
{
    Z80DebugState s = CPC::z80.GetDebugState();
    char buf[16];
    auto setHex = [&](QLineEdit *field, unsigned value, int width) {
        snprintf(buf, sizeof(buf), "%0*X", width, value);
        field->setText(buf);
    };
    setHex(ui->txtAF, s.AF, 4);
    setHex(ui->txtBC, s.BC, 4);
    setHex(ui->txtDE, s.DE, 4);
    setHex(ui->txtHL, s.HL, 4);
    setHex(ui->txtAF_, s.AF_, 4);
    setHex(ui->txtBC_, s.BC_, 4);
    setHex(ui->txtDE_, s.DE_, 4);
    setHex(ui->txtHL_, s.HL_, 4);
    setHex(ui->txtIX, s.IX, 4);
    setHex(ui->txtIY, s.IY, 4);
    setHex(ui->txtPC, s.PC, 4);
    setHex(ui->txtSP, s.SP, 4);
    setHex(ui->txtI,  s.I,  2);
    setHex(ui->txtR,  s.R,  2);

    ui->chkFlagS->setChecked(s.fS);
    ui->chkFlagZ->setChecked(s.fZ);
    ui->chkFlag5->setChecked(s.f5);
    ui->chkFlagH->setChecked(s.fH);
    ui->chkFlag3->setChecked(s.f3);
    ui->chkFlagP->setChecked(s.fP);
    ui->chkFlagN->setChecked(s.fN);
    ui->chkFlagC->setChecked(s.fC);

    ui->txtIM->setText(QString::number(s.im));
    ui->txtIRQ->setText(s.InterruptRequest ? "1" : "0");
    ui->txtIFF1->setText(s.IFF1 ? "1" : "0");
    ui->txtIFF2->setText(s.IFF2 ? "1" : "0");
    ui->txtNOPS->setText(QString::number(s.nops));
}

void Debugger::onZ80FieldEdited()
{
    Z80DebugState s = CPC::z80.GetDebugState();
    auto hex = [](QLineEdit *f, word fallback, unsigned mask) -> word {
        bool ok;
        unsigned v = f->text().toUInt(&ok, 16);
        return ok ? (word)(v & mask) : fallback;
    };
    auto bit = [](QLineEdit *f, bool fallback) -> bool {
        bool ok;
        unsigned v = f->text().toUInt(&ok, 10);
        return ok ? (v != 0) : fallback;
    };
    s.AF  = hex(ui->txtAF,  s.AF,  0xFFFF);
    s.BC  = hex(ui->txtBC,  s.BC,  0xFFFF);
    s.DE  = hex(ui->txtDE,  s.DE,  0xFFFF);
    s.HL  = hex(ui->txtHL,  s.HL,  0xFFFF);
    s.AF_ = hex(ui->txtAF_, s.AF_, 0xFFFF);
    s.BC_ = hex(ui->txtBC_, s.BC_, 0xFFFF);
    s.DE_ = hex(ui->txtDE_, s.DE_, 0xFFFF);
    s.HL_ = hex(ui->txtHL_, s.HL_, 0xFFFF);
    s.IX  = hex(ui->txtIX,  s.IX,  0xFFFF);
    s.IY  = hex(ui->txtIY,  s.IY,  0xFFFF);
    s.PC  = hex(ui->txtPC,  s.PC,  0xFFFF);
    s.SP  = hex(ui->txtSP,  s.SP,  0xFFFF);
    s.I   = (BYTE)hex(ui->txtI, s.I, 0xFF);
    s.R   = (BYTE)hex(ui->txtR, s.R, 0xFF);
    bool ok;
    unsigned imVal = ui->txtIM->text().toUInt(&ok, 10);
    if (ok && imVal <= 2) s.im = (BYTE)imVal;
    s.InterruptRequest = bit(ui->txtIRQ,  s.InterruptRequest);
    s.IFF1             = bit(ui->txtIFF1, s.IFF1);
    s.IFF2             = bit(ui->txtIFF2, s.IFF2);
    CPC::z80.SetDebugState(s);
    UpdateZ80Panel();
}

void Debugger::onCRTCFieldEdited()
{
    CRTC &c = CPC::crtc;
    auto dec = [](QLineEdit *f, BYTE fallback) -> BYTE {
        bool ok;
        unsigned v = f->text().toUInt(&ok, 10);
        return ok ? (BYTE)(v & 0xFF) : fallback;
    };
    auto hex16 = [](QLineEdit *f, word fallback) -> word {
        bool ok;
        unsigned v = f->text().toUInt(&ok, 16);
        return ok ? (word)(v & 0xFFFF) : fallback;
    };
    c.HT   = dec(ui->txtHT,   c.HT);
    c.HCC  = dec(ui->txtHCC,  c.HCC);
    c.HD   = dec(ui->txtHD,   c.HD);
    c.HSP  = dec(ui->txtHSP,  c.HSP);
    c.HSW  = dec(ui->txtHSW,  c.HSW);
    c.HSC  = dec(ui->txtHSC,  c.HSC);
    c.VSW  = dec(ui->txtVSW,  c.VSW);
    c.VSC  = dec(ui->txtVSC,  c.VSC);
    c.VT   = dec(ui->txtVT,   c.VT);
    c.VCC  = dec(ui->txtVCC,  c.VCC);
    c.VD   = dec(ui->txtVD,   c.VD);
    c.VSP  = dec(ui->txtVSP,  c.VSP);
    c.MRA  = dec(ui->txtMRA,  c.MRA);
    c.RA   = dec(ui->txtRA,   c.RA);
    c.VTA  = dec(ui->txtVTA,  c.VTA);
    c.VTAC = dec(ui->txtVTAC, c.VTAC);
    c.DSA  = hex16(ui->txtDSA, c.DSA);
    c.MA   = hex16(ui->txtMA,  c.MA);
    UpdateCRTCPanel();
}

void Debugger::UpdateStackPanel()
{
    QStringList entries;
    char buff[32];
    word sp = CPC::z80.GetSP();
    for (int i = 0; i < 256; i++)
    {
        BYTE L = CPC::GetByteAt(sp);
        BYTE H = CPC::GetByteAt(sp + 1);
        snprintf(buff, sizeof(buff), "%04X:%04X", sp, L + H * 256);
        entries.append(buff);
        sp += 2;
    }
    modelStack->setStringList(entries);
}

void Debugger::onStackClicked(const QModelIndex &index)
{
    word sp = (word)(CPC::z80.GetSP() + 2 * index.row());
    BYTE L = CPC::GetByteAt(sp);
    BYTE H = CPC::GetByteAt(sp + 1);
    word target = L + H * 256;

    int row = disassemblyAddresses.indexOf(target);
    if (row < 0)
    {
        manualAnchors.insert(target);
        Update();
        row = disassemblyAddresses.indexOf(target);
    }
    if (row >= 0)
    {
        QModelIndex mi = modelDisassembly->index(row);
        ui->listDisassembly->setCurrentIndex(mi);
        ui->listDisassembly->scrollTo(mi, QAbstractItemView::PositionAtCenter);
    }
}

void Debugger::UpdateCRTCPanel()
{
    CRTC &c = CPC::crtc;
    char buf[8];
    auto setDec = [&](QLineEdit *field, int value) {
        snprintf(buf, sizeof(buf), "%d", value);
        field->setText(buf);
    };
    auto setHex4 = [&](QLineEdit *field, unsigned value) {
        snprintf(buf, sizeof(buf), "%04X", value);
        field->setText(buf);
    };
    setDec(ui->txtHT, c.HT);
    setDec(ui->txtHCC, c.HCC);
    setDec(ui->txtHD, c.HD);
    ui->chkHDISP->setChecked(c.HDISP);
    setDec(ui->txtHSP, c.HSP);
    ui->chkHSYNC->setChecked(c.HSYNC);
    setDec(ui->txtHSW, c.HSW);
    setDec(ui->txtHSC, c.HSC);
    setDec(ui->txtVSW, c.VSW);
    setDec(ui->txtVSC, c.VSC);
    setDec(ui->txtVT, c.VT);
    setDec(ui->txtVCC, c.VCC);
    setDec(ui->txtVD, c.VD);
    ui->chkVDISP->setChecked(c.VDISP);
    setDec(ui->txtVSP, c.VSP);
    ui->chkVSYNC->setChecked(c.VSYNC);
    setDec(ui->txtMRA, c.MRA);
    setDec(ui->txtRA, c.RA);
    setDec(ui->txtVTA, c.VTA);
    setDec(ui->txtVTAC, c.VTAC);
    setHex4(ui->txtDSA, c.DSA);
    setHex4(ui->txtMA, c.MA);
    setDec(ui->txtHPos, CPC::screen.hPos);
    setDec(ui->txtVPos, CPC::screen.vPos);

    auto hilite = [](QWidget *w, bool on) {
        w->setStyleSheet(on ? "background-color: #505010; color: white;" : "");
    };
    hilite(ui->txtHT,  c.Index == 0);
    hilite(ui->txtHD,  c.Index == 1);
    hilite(ui->txtHSP, c.Index == 2);
    hilite(ui->txtHSW, c.Index == 3);
    hilite(ui->txtVSW, c.Index == 3);
    hilite(ui->txtVT,  c.Index == 4);
    hilite(ui->txtVTA, c.Index == 5);
    hilite(ui->txtVD,  c.Index == 6);
    hilite(ui->txtVSP, c.Index == 7);
    hilite(ui->txtMRA, c.Index == 9);
    hilite(ui->txtDSA, c.Index == 12 || c.Index == 13);
}

void Debugger::UpdateGateArrayPanel()
{
    GateArrayDebugState g = CPC::gateArray.GetDebugState();
    char buf[16];
    auto setDec = [&](QLineEdit *field, int value) {
        snprintf(buf, sizeof(buf), "%d", value);
        field->setText(buf);
    };
    auto setHex4 = [&](QLineEdit *field, unsigned value) {
        snprintf(buf, sizeof(buf), "%04X", value);
        field->setText(buf);
    };
    setDec(ui->txtPen, g.currentPen);
    setDec(ui->txtBorder, g.BORDER);
    setDec(ui->txtMode, g.mode);
    setHex4(ui->txtVideoAddr, g.videoAddress);
    QLabel *inks[16] = {
        ui->txtInk0, ui->txtInk1, ui->txtInk2, ui->txtInk3,
        ui->txtInk4, ui->txtInk5, ui->txtInk6, ui->txtInk7,
        ui->txtInk8, ui->txtInk9, ui->txtInk10, ui->txtInk11,
        ui->txtInk12, ui->txtInk13, ui->txtInk14, ui->txtInk15
    };
    for (int i = 0; i < 16; i++) {
        snprintf(buf, sizeof(buf), "%02X", g.INK[i] + 0x40);
        inks[i]->setText(buf);
        const BYTE *rgb = CPC::gateArray.GetPaletteEntry(i);
        const char *fg = (rgb[0] * 299 + rgb[1] * 587 + rgb[2] * 114) < 128000 ? "white" : "black";
        inks[i]->setStyleSheet(QString("background-color: rgb(%1, %2, %3); color: %4; border: 1px solid #808080; font-family: \"Ubuntu Sans Mono\"; font-size: 9pt;")
                               .arg(rgb[0]).arg(rgb[1]).arg(rgb[2]).arg(fg));
    }
    ui->chkLoR->setChecked(g.LoROMActive);
    ui->chkHiR->setChecked(g.HiROMActive);
    ui->chkCartridge->setChecked(CPC::cartridgeEnabled);
    setDec(ui->txtR52, g.R52);
    BYTE cw = CPC::ppi.controlWord;
    char binBuf[9];
    for (int i = 0; i < 8; i++) binBuf[i] = (cw & (0x80 >> i)) ? '1' : '0';
    binBuf[8] = 0;
    ui->txtPPI->setText(binBuf);
}

void Debugger::PopulateMemorySources()
{
    QSignalBlocker blocker(ui->cmbMemorySource);
    ui->cmbMemorySource->clear();
    ui->cmbMemorySource->addItem("CPU view",           int(CpuView));
    ui->cmbMemorySource->addItem("RAM (current map)",  int(RamCurrent));
    ui->cmbMemorySource->addItem("RAM bank",           int(RamBank));
    ui->cmbMemorySource->addItem("Lower ROM",          int(LowerRom));
    ui->cmbMemorySource->addItem("Upper ROM slot",     int(UpperRomSlot));
    ui->cmbMemorySource->addItem("Cartridge",          int(Cartridge));
    ui->cmbMemorySource->setCurrentIndex(0);
}

void Debugger::PopulateMemoryDetail()
{
    QSignalBlocker blocker(ui->cmbMemoryDetail);
    ui->cmbMemoryDetail->clear();
    switch (memSource)
    {
    case RamBank: {
        int count = (CPC::cpcType == CPCType::CPC6128) ? 8 : 4;
        for (int i = 0; i < count; i++)
            ui->cmbMemoryDetail->addItem(QString("Bank %1").arg(i), i);
        ui->cmbMemoryDetail->setEnabled(true);
        break;
    }
    case UpperRomSlot: {
        for (int i = 0; i < ROM_SLOTS; i++)
            ui->cmbMemoryDetail->addItem(QString("Slot %1%2").arg(i).arg(CPC::HiROMs[i] ? "" : " (empty)"), i);
        ui->cmbMemoryDetail->setEnabled(true);
        break;
    }
    case Cartridge: {
        for (int i = 0; i < 32; i++)
            ui->cmbMemoryDetail->addItem(QString("Bank #%1").arg(0x80 + i, 2, 16, QChar('0')).toUpper(), i);
        ui->cmbMemoryDetail->setEnabled(true);
        break;
    }
    default:
        ui->cmbMemoryDetail->setEnabled(false);
        break;
    }
    if (memDetail >= 0 && memDetail < ui->cmbMemoryDetail->count())
        ui->cmbMemoryDetail->setCurrentIndex(memDetail);
    else if (ui->cmbMemoryDetail->count() > 0)
        ui->cmbMemoryDetail->setCurrentIndex(0);
}

void Debugger::onMemorySourceChanged(int index)
{
    if (index < 0) return;
    memSource = MemSource(ui->cmbMemorySource->itemData(index).toInt());
    memDetail = 0;
    PopulateMemoryDetail();
    Update();
}

void Debugger::onMemoryDetailChanged(int index)
{
    if (index < 0) return;
    memDetail = ui->cmbMemoryDetail->itemData(index).toInt();
    Update();
}

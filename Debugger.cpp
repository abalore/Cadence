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
#include <QStringListModel>
#include <QModelIndex>
#include <QScrollBar>
#include <QShortcut>
#include <QInputDialog>
#include <algorithm>

Debugger::Debugger(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Debugger)
{
    ui->setupUi(this);
    connect(ui->btnStepOut, &QPushButton::clicked, this, &Debugger::onStepOutClicked);
    connect(ui->btnRun, &QPushButton::clicked, this, &Debugger::onRunClicked);
    connect(ui->btnStepOver, &QPushButton::clicked, this, &Debugger::onStepOverClicked);
    connect(ui->btnStepIn, &QPushButton::clicked, this, &Debugger::onStepInClicked);
    connect(ui->btnRunTo, &QPushButton::clicked, this, &Debugger::onRunToClicked);
    connect(ui->btnResetNops, &QPushButton::clicked, this, &Debugger::onResetNopsClicked);
    connect(ui->btnToggleBreakpoint, &QPushButton::clicked, this, &Debugger::onToggleBreakpointClicked);
    connect(ui->btnGoTo, &QPushButton::clicked, this, &Debugger::onGoToClicked);

    modelDisassembly = new QStringListModel();
    ui->listDisassembly->setModel(modelDisassembly);
    modelMemory = new QStringListModel();
    ui->listMemory->setModel(modelMemory);
    connect(new QShortcut(Qt::Key_F6, this), &QShortcut::activated, this, &Debugger::onStepOutClicked);
    connect(new QShortcut(Qt::Key_F5, this), &QShortcut::activated, this, &Debugger::onRunClicked);
    connect(new QShortcut(Qt::Key_F8, this), &QShortcut::activated, this, &Debugger::onStepOverClicked);
    connect(new QShortcut(Qt::Key_F7, this), &QShortcut::activated, this, &Debugger::onStepInClicked);
    connect(new QShortcut(Qt::Key_F4, this), &QShortcut::activated, this, &Debugger::onRunToClicked);
    connect(new QShortcut(Qt::Key_F9, this), &QShortcut::activated, this, &Debugger::onToggleBreakpointClicked);
}

Debugger::~Debugger()
{
    delete modelMemory;
    delete modelDisassembly;
    delete ui;
}

void Debugger::closeEvent(QCloseEvent *event)
{
    EmulatorThread::Run();
    setHidden(true);
    event->ignore();
}

void Debugger::Update()
{
    int pcPosition = 0;
    int instrCount = 0;
    listDisassembly.clear();
    disassemblyAddresses.clear();

    // Build the sorted anchor list: manual anchors + current PC + breakpoints.
    std::vector<int> anchors(manualAnchors.begin(), manualAnchors.end());
    anchors.push_back(CPC::z80.GetPC());
    for (int i = 0; i < 65536; i++)
        if (CPC::Breakpoint[i]) anchors.push_back(i);
    anchors.push_back(0x10000);
    std::sort(anchors.begin(), anchors.end());
    anchors.erase(std::unique(anchors.begin(), anchors.end()), anchors.end());

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
        sprintf(buff, "%s %14s  %4s  %12s  %s", CPC::Breakpoint[position] ? "* " : "  ", label.data(), address.data(), bytes.data(), instruction.data());
        listDisassembly.append(buff);
        disassemblyAddresses.append(position);
        if (!pcFound && position >= CPC::z80.GetPC())
        {
            pcPosition = instrCount;
            pcFound = true;
            nextInstructionLength = instrLength;
            nextInstructionOpCode = opCode;
        }
        instrCount++;
    }
    modelDisassembly -> setStringList(listDisassembly);
    modelDisassemblyIndex = modelDisassembly->index(pcPosition);
    ui->listDisassembly->scrollTo(modelDisassemblyIndex, QAbstractItemView::PositionAtCenter);
    ui->listDisassembly->setCurrentIndex(modelDisassemblyIndex);

    int memoryIndex = ui->listMemory->currentIndex().row();
    listMemory.clear();
    for (int i = 0x0000; i <= 0xFFF0; i += 16)
    {
        int bank = i >> 14;
        int address = i & 0x3FFF;
        BYTE *mem = CPC::RAM[bank];
        sprintf(buff, "%04X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
                i, mem[address], mem[address + 1],
                mem[address + 2], mem[address + 3],
                mem[address + 4], mem[address + 5],
                mem[address + 6], mem[address + 7],
                mem[address + 8], mem[address + 9],
                mem[address + 10], mem[address + 11],
                mem[address + 12], mem[address + 13],
                mem[address + 14], mem[address + 15]);
        listMemory.append(buff);
    }

    modelMemory -> setStringList(listMemory);
    modelMemoryIndex = modelMemory->index(memoryIndex);
    ui->listMemory->scrollTo(modelMemoryIndex, QAbstractItemView::PositionAtCenter);
    ui->listMemory->setCurrentIndex(modelMemoryIndex);

    UpdateZ80Panel();
    UpdateCRTCPanel();
    UpdateGateArrayPanel();
    ui->lblStack->setText(GetZ80StackDebugLine().data());

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

string Debugger::GetZ80StackDebugLine()
{
    string d;
    char buff[100];
    word sp = CPC::z80.GetSP();
    for (int i = 0; i < 8; i++)
    {
        BYTE L = CPC::GetByteAt(sp);
        BYTE H = CPC::GetByteAt(sp + 1);
        sprintf(buff, "%04X:%04X\n", sp, L + H * 256);
        d.append(buff);
        sp += 2;
    }
    return d;
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

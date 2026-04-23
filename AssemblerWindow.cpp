#include "AssemblerWindow.h"

#include "CPC.h"
#include "EmulatorThread.h"
#include "mainwindow.h"

#include <QCloseEvent>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QHash>
#include <QFontMetrics>
#include <QFontDatabase>
#include <QKeySequence>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QTextDocument>
#include <QStatusBar>
#include <QTextStream>
#include <QThread>
#include <QVBoxLayout>
#include <QWidget>

AssemblerWindow::AssemblerWindow(QWidget *parent)
    : QMainWindow(parent)
    , dirty(false)
{
    setWindowTitle(tr("Assembler"));
    resize(900, 700);

    QFont mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    mono.setPointSize(11);

    source = new QPlainTextEdit(this);
    source->setFont(mono);
    source->setTabStopDistance(QFontMetrics(mono).horizontalAdvance(' ') * 8);
    source->setPlaceholderText(tr("; Type Z80 assembly here\n; Example:\n;   ORG &8000\n;   LD A,42\n;   RET\n"));

    output = new QPlainTextEdit(this);
    output->setFont(mono);
    output->setReadOnly(true);
    output->setPlaceholderText(tr("Assembly output will appear here."));

    QSplitter *split = new QSplitter(Qt::Vertical, this);
    split->addWidget(source);
    split->addWidget(output);
    split->setStretchFactor(0, 3);
    split->setStretchFactor(1, 1);
    setCentralWidget(split);

    actNew       = new QAction(tr("&New"), this);
    actOpen      = new QAction(tr("&Open..."), this);
    actSave      = new QAction(tr("&Save"), this);
    actSaveAs    = new QAction(tr("Save &As..."), this);
    actUndo      = new QAction(tr("&Undo"), this);
    actRedo      = new QAction(tr("&Redo"), this);
    actCut       = new QAction(tr("Cu&t"), this);
    actCopy      = new QAction(tr("&Copy"), this);
    actPaste     = new QAction(tr("&Paste"), this);
    actSelectAll = new QAction(tr("Select &All"), this);
    actAssemble  = new QAction(tr("&Assemble"), this);

    actUndo->setShortcuts(QKeySequence::Undo);
    actRedo->setShortcuts(QKeySequence::Redo);
    actCut->setShortcuts(QKeySequence::Cut);
    actCopy->setShortcuts(QKeySequence::Copy);
    actPaste->setShortcuts(QKeySequence::Paste);
    actSelectAll->setShortcuts(QKeySequence::SelectAll);

    actUndo->setEnabled(false);
    actRedo->setEnabled(false);
    actCut->setEnabled(false);
    actCopy->setEnabled(false);

    QMenuBar *mb = menuBar();
    QMenu *fileMenu = mb->addMenu(tr("&File"));
    fileMenu->addAction(actNew);
    fileMenu->addAction(actOpen);
    fileMenu->addSeparator();
    fileMenu->addAction(actSave);
    fileMenu->addAction(actSaveAs);
    QMenu *editMenu = mb->addMenu(tr("&Edit"));
    editMenu->addAction(actUndo);
    editMenu->addAction(actRedo);
    editMenu->addSeparator();
    editMenu->addAction(actCut);
    editMenu->addAction(actCopy);
    editMenu->addAction(actPaste);
    editMenu->addSeparator();
    editMenu->addAction(actSelectAll);
    QMenu *buildMenu = mb->addMenu(tr("&Build"));
    buildMenu->addAction(actAssemble);

    connect(actNew,      &QAction::triggered, this, &AssemblerWindow::onNew);
    connect(actOpen,     &QAction::triggered, this, &AssemblerWindow::onOpen);
    connect(actSave,     &QAction::triggered, this, &AssemblerWindow::onSave);
    connect(actSaveAs,   &QAction::triggered, this, &AssemblerWindow::onSaveAs);
    connect(actAssemble, &QAction::triggered, this, &AssemblerWindow::onAssemble);
    connect(source->document(), &QTextDocument::modificationChanged, this, &AssemblerWindow::onSourceModified);

    connect(actUndo,      &QAction::triggered, source, &QPlainTextEdit::undo);
    connect(actRedo,      &QAction::triggered, source, &QPlainTextEdit::redo);
    connect(actCut,       &QAction::triggered, source, &QPlainTextEdit::cut);
    connect(actCopy,      &QAction::triggered, source, &QPlainTextEdit::copy);
    connect(actPaste,     &QAction::triggered, source, &QPlainTextEdit::paste);
    connect(actSelectAll, &QAction::triggered, source, &QPlainTextEdit::selectAll);
    connect(source, &QPlainTextEdit::undoAvailable, actUndo, &QAction::setEnabled);
    connect(source, &QPlainTextEdit::redoAvailable, actRedo, &QAction::setEnabled);
    connect(source, &QPlainTextEdit::copyAvailable, actCut,  &QAction::setEnabled);
    connect(source, &QPlainTextEdit::copyAvailable, actCopy, &QAction::setEnabled);

    statusBar()->showMessage(tr("Ready"));
    updateTitle();
}

AssemblerWindow::~AssemblerWindow() = default;

void AssemblerWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

void AssemblerWindow::onSourceModified(bool modified)
{
    dirty = modified;
    updateTitle();
}

void AssemblerWindow::onNew()
{
    if (!maybeSave()) return;
    source->clear();
    source->document()->setModified(false);
    setCurrentFile(QString());
    clearOutput();
}

void AssemblerWindow::onOpen()
{
    if (!maybeSave()) return;
    QString path = QFileDialog::getOpenFileName(this, tr("Open Source"), QString(),
                                                tr("Assembly files (*.asm *.z80 *.s);;All files (*)"));
    if (path.isEmpty()) return;
    if (readFromFile(path))
    {
        setCurrentFile(path);
        clearOutput();
    }
}

void AssemblerWindow::onSave()
{
    if (currentFile.isEmpty())
    {
        onSaveAs();
        return;
    }
    writeToFile(currentFile);
}

void AssemblerWindow::onSaveAs()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Save Source"), currentFile,
                                                tr("Assembly files (*.asm *.z80 *.s);;All files (*)"));
    if (path.isEmpty()) return;
    if (writeToFile(path))
        setCurrentFile(path);
}

void AssemblerWindow::onAssemble()
{
    clearOutput();
    AssemblerResult r = assembler.Assemble(source->toPlainText());
    for (const AssemblerMessage &m : r.messages)
    {
        QString prefix = m.isError ? tr("error") : tr("info");
        QString line = m.line > 0 ? QString("(%1) ").arg(m.line) : QString();
        appendOutput(QString("%1%2: %3").arg(line, prefix, m.text), m.isError);
    }
    if (!r.ok)
    {
        statusBar()->showMessage(tr("Assembly failed"), 5000);
        return;
    }

    if (EmulatorThread::running)
    {
        EmulatorThread::Pause();
        QElapsedTimer t;
        t.start();
        while (EmulatorThread::running && t.elapsed() < 500)
            QThread::msleep(2);
        if (EmulatorThread::running)
        {
            appendOutput(tr("Failed to pause emulator — RAM not updated."), true);
            statusBar()->showMessage(tr("Assemble: pause timeout"), 5000);
            return;
        }
    }

    auto ramForBank = [](int ramBank, int bankIdx) -> BYTE * {
        int cfg = ramBank & 0x07;
        static const int map[8][4] = {
            {0, 1, 2, 3}, {0, 1, 2, 7}, {4, 5, 6, 7}, {0, 3, 2, 7},
            {0, 4, 2, 3}, {0, 5, 2, 3}, {0, 6, 2, 3}, {0, 7, 2, 3},
        };
        int slot = map[cfg][bankIdx];
        return CPC::RAMs[slot];
    };

    int total = 0;
    int fileTotal = 0;
    bool anyFailed = false;
    QHash<QString, QByteArray> fileBuffers;
    QStringList fileOrder;
    for (const AssemblerSegment &s : r.segments)
    {
        if (!s.fileName.isEmpty())
        {
            if (!fileBuffers.contains(s.fileName))
                fileOrder.append(s.fileName);
            fileBuffers[s.fileName].append(s.bytes);
            fileTotal += s.bytes.size();
            appendOutput(QString("emitted %1 bytes to file \"%2\"").arg(s.bytes.size()).arg(s.fileName), false);
            continue;
        }
        int written = 0, skipped = 0;
        for (int i = 0; i < s.bytes.size(); i++)
        {
            word addr = word((s.writeOrigin + i) & 0xFFFF);
            int bankIdx = addr >> 14;
            int off = addr & 0x3FFF;
            BYTE b = BYTE(uchar(s.bytes[i]));
            BYTE *dst = nullptr;
            if (bankIdx == 0 && s.lowerRom == 0)
                dst = CPC::LoROM;
            else if (bankIdx == 3 && s.upperRom >= 0)
            {
                if (s.upperRom >= 0x80 && s.upperRom <= 0x9F)
                {
                    if (CPC::cartridgeEnabled && CPC::Cartridge)
                        dst = CPC::Cartridge + (s.upperRom - 0x80) * 0x4000;
                }
                else
                    dst = CPC::HiROMs[s.upperRom];
            }
            else
                dst = ramForBank(s.ramBank, bankIdx);
            if (dst) { dst[off] = b; written++; }
            else { skipped++; }
        }
        QString addr = QString::number(s.writeOrigin, 16).rightJustified(4, '0').toUpper();
        if (s.writeOrigin != s.origin)
        {
            QString orgStr = QString::number(s.origin, 16).rightJustified(4, '0').toUpper();
            addr = QString("%1 (org &%2)").arg(addr, orgStr);
        }
        auto upperLabel = [](int n) -> QString {
            if (n >= 0x80 && n <= 0x9F)
                return QString("CART#%1").arg(n - 0x80);
            return QString("UpROM#%1").arg(n);
        };
        QString tgt;
        if (s.lowerRom == 0 && s.upperRom >= 0)
            tgt = QString("LoROM+%1").arg(upperLabel(s.upperRom));
        else if (s.lowerRom == 0)
            tgt = QString("LoROM+RAM&%1").arg(QString::number(s.ramBank, 16).toUpper());
        else if (s.upperRom >= 0)
            tgt = QString("%1+RAM&%2").arg(upperLabel(s.upperRom), QString::number(s.ramBank, 16).toUpper());
        else
            tgt = QString("RAM&%1").arg(QString::number(s.ramBank, 16).toUpper());
        appendOutput(QString("poked %1 bytes at &%2 [%3]").arg(written).arg(addr).arg(tgt), false);
        if (skipped > 0)
        {
            appendOutput(QString("  %1 bytes skipped: target buffer not available (ROM slot empty or RAM bank beyond installed memory)").arg(skipped), true);
            anyFailed = true;
        }
        total += written;
    }
    for (const QString &fname : fileOrder)
    {
        QString fullPath = fname;
        if (QDir::isRelativePath(fname))
        {
            QString binDir = QDir::homePath() + "/.cadence/BIN";
            QDir().mkpath(binDir);
            fullPath = binDir + "/" + fname;
        }
        QFile f(fullPath);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            appendOutput(QString("failed to open \"%1\" for writing: %2").arg(fullPath, f.errorString()), true);
            anyFailed = true;
            continue;
        }
        const QByteArray &buf = fileBuffers[fname];
        qint64 w = f.write(buf);
        f.close();
        if (w != buf.size())
        {
            appendOutput(QString("short write to \"%1\" (%2 of %3 bytes)").arg(fullPath).arg(w).arg(buf.size()), true);
            anyFailed = true;
        }
        else
            appendOutput(QString("wrote %1 bytes to %2").arg(buf.size()).arg(fullPath), false);
    }
    if (fileTotal > 0)
        appendOutput(tr("Assembled %1 bytes into emulator memory and %2 bytes to file(s). Emulator is paused.").arg(total).arg(fileTotal), false);
    else
        appendOutput(tr("Assembled %1 bytes into emulator memory. Emulator is paused.").arg(total), false);
    if (anyFailed)
        statusBar()->showMessage(tr("Assembled with skipped bytes (paused)"), 5000);
    else
        statusBar()->showMessage(tr("Assembled to memory (paused)"), 5000);

    if (MainWindow::Instance)
        MainWindow::Instance->RefreshDebuggerIfOpen();
}

bool AssemblerWindow::maybeSave()
{
    if (!dirty) return true;
    QMessageBox::StandardButton ret = QMessageBox::warning(
        this, tr("Assembler"),
        tr("The source has been modified.\nDo you want to save your changes?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
    {
        onSave();
        return !dirty;
    }
    if (ret == QMessageBox::Discard)
    {
        source->document()->setModified(false);
        return true;
    }
    return false;
}

bool AssemblerWindow::writeToFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("Assembler"),
                             tr("Cannot write %1:\n%2").arg(path, f.errorString()));
        return false;
    }
    QTextStream out(&f);
    out << source->toPlainText();
    source->document()->setModified(false);
    statusBar()->showMessage(tr("Saved %1").arg(QFileInfo(path).fileName()), 3000);
    return true;
}

bool AssemblerWindow::readFromFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("Assembler"),
                             tr("Cannot read %1:\n%2").arg(path, f.errorString()));
        return false;
    }
    QTextStream in(&f);
    source->setPlainText(in.readAll());
    source->document()->setModified(false);
    return true;
}

void AssemblerWindow::setCurrentFile(const QString &path)
{
    currentFile = path;
    dirty = false;
    updateTitle();
}

void AssemblerWindow::updateTitle()
{
    QString name = currentFile.isEmpty() ? tr("Untitled") : QFileInfo(currentFile).fileName();
    setWindowTitle(tr("%1%2 — Assembler").arg(dirty ? "*" : "", name));
}

void AssemblerWindow::appendOutput(const QString &text, bool isError)
{
    (void)isError;
    output->appendPlainText(text);
}

void AssemblerWindow::clearOutput()
{
    output->clear();
}

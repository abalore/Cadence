#include "AssemblerWindow.h"

#include "AsmHighlighter.h"
#include "CPC.h"
#include "EmulatorThread.h"
#include "mainwindow.h"

#include <QCloseEvent>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontMetrics>
#include <QFontDatabase>
#include <QKeySequence>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QTabWidget>
#include <QTextDocument>
#include <QStatusBar>
#include <QTextStream>
#include <QThread>
#include <QVBoxLayout>
#include <QWidget>

namespace {
const char *kFilePropName = "asm_currentFile";
}

AssemblerWindow::AssemblerWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Assembler"));
    resize(900, 700);

    monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    monoFont.setPointSize(11);

    tabs = new QTabWidget(this);
    tabs->setDocumentMode(true);
    tabs->setTabsClosable(true);
    tabs->setMovable(true);
    connect(tabs, &QTabWidget::tabCloseRequested, this, &AssemblerWindow::onTabCloseRequested);
    connect(tabs, &QTabWidget::currentChanged, this, &AssemblerWindow::onTabChanged);

    output = new QPlainTextEdit(this);
    output->setFont(monoFont);
    output->setReadOnly(true);
    output->setPlaceholderText(tr("Assembly output will appear here."));

    QSplitter *split = new QSplitter(Qt::Vertical, this);
    split->addWidget(tabs);
    split->addWidget(output);
    split->setStretchFactor(0, 3);
    split->setStretchFactor(1, 1);
    setCentralWidget(split);

    actNew       = new QAction(tr("&New"), this);
    actOpen      = new QAction(tr("&Open..."), this);
    actSave      = new QAction(tr("&Save"), this);
    actSaveAs    = new QAction(tr("Save &As..."), this);
    actCloseTab  = new QAction(tr("&Close Tab"), this);
    actUndo      = new QAction(tr("&Undo"), this);
    actRedo      = new QAction(tr("&Redo"), this);
    actCut       = new QAction(tr("Cu&t"), this);
    actCopy      = new QAction(tr("&Copy"), this);
    actPaste     = new QAction(tr("&Paste"), this);
    actSelectAll = new QAction(tr("Select &All"), this);
    actAssemble  = new QAction(tr("&Assemble"), this);

    actNew->setShortcuts(QKeySequence::New);
    actOpen->setShortcuts(QKeySequence::Open);
    actSave->setShortcuts(QKeySequence::Save);
    actSaveAs->setShortcuts(QKeySequence::SaveAs);
    actCloseTab->setShortcuts(QKeySequence::Close);
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
    fileMenu->addSeparator();
    fileMenu->addAction(actCloseTab);
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
    connect(actCloseTab, &QAction::triggered, this, &AssemblerWindow::onCloseTab);
    connect(actAssemble, &QAction::triggered, this, &AssemblerWindow::onAssemble);

    connect(actUndo,      &QAction::triggered, this, [this]() { if (auto *e = currentEditor()) e->undo(); });
    connect(actRedo,      &QAction::triggered, this, [this]() { if (auto *e = currentEditor()) e->redo(); });
    connect(actCut,       &QAction::triggered, this, [this]() { if (auto *e = currentEditor()) e->cut(); });
    connect(actCopy,      &QAction::triggered, this, [this]() { if (auto *e = currentEditor()) e->copy(); });
    connect(actPaste,     &QAction::triggered, this, [this]() { if (auto *e = currentEditor()) e->paste(); });
    connect(actSelectAll, &QAction::triggered, this, [this]() { if (auto *e = currentEditor()) e->selectAll(); });

    newEditorTab();

    statusBar()->showMessage(tr("Ready"));
    updateTitle();
}

AssemblerWindow::~AssemblerWindow() = default;

QPlainTextEdit *AssemblerWindow::currentEditor() const
{
    return qobject_cast<QPlainTextEdit *>(tabs->currentWidget());
}

QString AssemblerWindow::editorFile(QPlainTextEdit *ed) const
{
    if (!ed) return QString();
    return ed->property(kFilePropName).toString();
}

void AssemblerWindow::setEditorFile(QPlainTextEdit *ed, const QString &path)
{
    if (!ed) return;
    ed->setProperty(kFilePropName, path);
    ed->document()->setModified(false);
    updateTabLabel(ed);
    if (ed == currentEditor()) updateTitle();
}

void AssemblerWindow::updateTabLabel(QPlainTextEdit *ed)
{
    if (!ed) return;
    int idx = tabs->indexOf(ed);
    if (idx < 0) return;
    QString path = editorFile(ed);
    QString name = path.isEmpty() ? tr("Untitled") : QFileInfo(path).fileName();
    bool dirty = ed->document()->isModified();
    tabs->setTabText(idx, dirty ? QString("*") + name : name);
    if (!path.isEmpty())
        tabs->setTabToolTip(idx, path);
    else
        tabs->setTabToolTip(idx, QString());
}

QPlainTextEdit *AssemblerWindow::newEditorTab(const QString &path)
{
    QPlainTextEdit *ed = new QPlainTextEdit;
    ed->setFont(monoFont);
    ed->setTabStopDistance(QFontMetrics(monoFont).horizontalAdvance(' ') * 8);
    ed->setPlaceholderText(tr("; Type Z80 assembly here\n; Example:\n;   ORG &8000\n;   LD A,42\n;   RET\n"));
    ed->setProperty(kFilePropName, path);
    new AsmHighlighter(ed->document());

    connect(ed->document(), &QTextDocument::modificationChanged, this, [this, ed]() {
        updateTabLabel(ed);
        if (ed == currentEditor()) updateTitle();
    });

    int idx = tabs->addTab(ed, QString());
    updateTabLabel(ed);
    tabs->setCurrentIndex(idx);
    ed->setFocus();
    return ed;
}

void AssemblerWindow::wireEditorSignals(QPlainTextEdit *ed)
{
    for (const auto &c : editorConnections) QObject::disconnect(c);
    editorConnections.clear();
    if (!ed)
    {
        actUndo->setEnabled(false);
        actRedo->setEnabled(false);
        actCut->setEnabled(false);
        actCopy->setEnabled(false);
        return;
    }
    editorConnections << connect(ed, &QPlainTextEdit::undoAvailable, actUndo, &QAction::setEnabled);
    editorConnections << connect(ed, &QPlainTextEdit::redoAvailable, actRedo, &QAction::setEnabled);
    editorConnections << connect(ed, &QPlainTextEdit::copyAvailable, actCut,  &QAction::setEnabled);
    editorConnections << connect(ed, &QPlainTextEdit::copyAvailable, actCopy, &QAction::setEnabled);
    actUndo->setEnabled(ed->document()->isUndoAvailable());
    actRedo->setEnabled(ed->document()->isRedoAvailable());
    actCut->setEnabled(ed->textCursor().hasSelection());
    actCopy->setEnabled(ed->textCursor().hasSelection());
}

void AssemblerWindow::onTabChanged(int index)
{
    Q_UNUSED(index);
    wireEditorSignals(currentEditor());
    updateTitle();
}

void AssemblerWindow::closeEvent(QCloseEvent *event)
{
    for (int i = 0; i < tabs->count(); i++)
    {
        QPlainTextEdit *ed = qobject_cast<QPlainTextEdit *>(tabs->widget(i));
        if (!ed) continue;
        tabs->setCurrentIndex(i);
        if (!maybeSaveEditor(ed)) { event->ignore(); return; }
    }
    event->accept();
}

void AssemblerWindow::onNew()
{
    newEditorTab();
    clearOutput();
}

void AssemblerWindow::onOpen()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open Source"), QString(),
                                                tr("Assembly files (*.asm *.z80 *.s);;All files (*)"),
                                                nullptr, QFileDialog::DontUseNativeDialog);
    if (path.isEmpty()) return;

    for (int i = 0; i < tabs->count(); i++)
    {
        QPlainTextEdit *e = qobject_cast<QPlainTextEdit *>(tabs->widget(i));
        if (e && editorFile(e) == path)
        {
            tabs->setCurrentIndex(i);
            return;
        }
    }

    QPlainTextEdit *ed = currentEditor();
    bool reuse = ed && editorFile(ed).isEmpty() && !ed->document()->isModified() && ed->document()->isEmpty();
    if (!reuse) ed = newEditorTab();

    if (readEditorFromFile(ed, path))
    {
        setEditorFile(ed, path);
        clearOutput();
    }
}

void AssemblerWindow::onSave()
{
    QPlainTextEdit *ed = currentEditor();
    if (!ed) return;
    QString path = editorFile(ed);
    if (path.isEmpty()) { onSaveAs(); return; }
    writeEditorToFile(ed, path);
}

void AssemblerWindow::onSaveAs()
{
    QPlainTextEdit *ed = currentEditor();
    if (!ed) return;
    QString path = QFileDialog::getSaveFileName(this, tr("Save Source"), editorFile(ed),
                                                tr("Assembly files (*.asm *.z80 *.s);;All files (*)"),
                                                nullptr, QFileDialog::DontUseNativeDialog);
    if (path.isEmpty()) return;
    if (writeEditorToFile(ed, path))
        setEditorFile(ed, path);
}

void AssemblerWindow::onCloseTab()
{
    int idx = tabs->currentIndex();
    if (idx >= 0) onTabCloseRequested(idx);
}

void AssemblerWindow::onTabCloseRequested(int index)
{
    QPlainTextEdit *ed = qobject_cast<QPlainTextEdit *>(tabs->widget(index));
    if (!ed) return;
    tabs->setCurrentIndex(index);
    if (!maybeSaveEditor(ed)) return;
    tabs->removeTab(index);
    ed->deleteLater();
    if (tabs->count() == 0) newEditorTab();
}

void AssemblerWindow::onAssemble()
{
    clearOutput();
    QPlainTextEdit *ed = currentEditor();
    if (!ed) return;
    QString path = editorFile(ed);
    QString base = path.isEmpty() ? QString() : QFileInfo(path).absolutePath();
    AssemblerResult r = assembler.Assemble(ed->toPlainText(), base);
    for (const AssemblerMessage &m : r.messages)
    {
        QString prefix = m.isError ? tr("error") : tr("info");
        QString loc;
        if (!m.source.isEmpty())
            loc = QString("(%1:%2) ").arg(QFileInfo(m.source).fileName(), QString::number(m.line));
        else if (m.line > 0)
            loc = QString("(%1) ").arg(m.line);
        appendOutput(QString("%1%2: %3").arg(loc, prefix, m.text), m.isError);
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

bool AssemblerWindow::maybeSaveEditor(QPlainTextEdit *ed)
{
    if (!ed || !ed->document()->isModified()) return true;
    QString path = editorFile(ed);
    QString name = path.isEmpty() ? tr("Untitled") : QFileInfo(path).fileName();
    QMessageBox::StandardButton ret = QMessageBox::warning(
        this, tr("Assembler"),
        tr("The file \"%1\" has been modified.\nDo you want to save your changes?").arg(name),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
    {
        QString target = path;
        if (target.isEmpty())
        {
            target = QFileDialog::getSaveFileName(this, tr("Save Source"), QString(),
                                                  tr("Assembly files (*.asm *.z80 *.s);;All files (*)"),
                                                  nullptr, QFileDialog::DontUseNativeDialog);
            if (target.isEmpty()) return false;
        }
        if (!writeEditorToFile(ed, target)) return false;
        setEditorFile(ed, target);
        return true;
    }
    if (ret == QMessageBox::Discard)
    {
        ed->document()->setModified(false);
        return true;
    }
    return false;
}

bool AssemblerWindow::writeEditorToFile(QPlainTextEdit *ed, const QString &path)
{
    QFile f(path);
    if (!f.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("Assembler"),
                             tr("Cannot write %1:\n%2").arg(path, f.errorString()));
        return false;
    }
    QTextStream out(&f);
    out << ed->toPlainText();
    ed->document()->setModified(false);
    updateTabLabel(ed);
    statusBar()->showMessage(tr("Saved %1").arg(QFileInfo(path).fileName()), 3000);
    return true;
}

bool AssemblerWindow::readEditorFromFile(QPlainTextEdit *ed, const QString &path)
{
    QFile f(path);
    if (!f.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("Assembler"),
                             tr("Cannot read %1:\n%2").arg(path, f.errorString()));
        return false;
    }
    QTextStream in(&f);
    ed->setPlainText(in.readAll());
    ed->document()->setModified(false);
    return true;
}

void AssemblerWindow::updateTitle()
{
    QPlainTextEdit *ed = currentEditor();
    QString path = editorFile(ed);
    QString name = path.isEmpty() ? tr("Untitled") : QFileInfo(path).fileName();
    bool dirty = ed && ed->document()->isModified();
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

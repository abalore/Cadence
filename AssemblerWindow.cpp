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
#include <QCoreApplication>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QSettings>
#include <QTextBlock>
#include <QTextCursor>
#include <QProgressBar>
#include <QSet>
#include <QSplitter>
#include <QTabWidget>
#include <QTextDocument>
#include <QStatusBar>
#include <QTextStream>
#include <QThread>
#include <QVBoxLayout>
#include <QWidget>
#include <cstring>

namespace {
const char *kFilePropName = "asm_currentFile";

struct AmsdosGeom
{
    bool valid = false;
    int reservedTracks = 0;
    int sectorBaseId = 0xC1;
    int sectorsPerTrack = 9;
    int sectorSize = 512;
    int blockSize = 1024;
    int dirSectors = 4;
    int dirEntries = 64;
    int maxBlock = 179;
};

static AmsdosGeom detectAmsdosGeom(FloppyDrive *drv)
{
    AmsdosGeom g;
    if (!drv || !drv->DiskInserted) return g;
    if (drv->GetSectorDataById(0, 0, 0xC1) != nullptr)
    {
        g.reservedTracks = 0;
        g.sectorBaseId = 0xC1;
        int tracks = drv->GetTracks();
        g.maxBlock = tracks * g.sectorsPerTrack * g.sectorSize / g.blockSize - 1;
        g.valid = true;
        return g;
    }
    if (drv->GetTracks() > 2 && drv->GetSectorDataById(2, 0, 0x41) != nullptr)
    {
        g.reservedTracks = 2;
        g.sectorBaseId = 0x41;
        int usable = drv->GetTracks() - 2;
        g.maxBlock = usable * g.sectorsPerTrack * g.sectorSize / g.blockSize - 1;
        g.valid = true;
        return g;
    }
    return g;
}

static void blockToSectors(const AmsdosGeom &g, int block,
                           int &tA, int &idA, int &tB, int &idB)
{
    int logA = block * 2;
    int logB = logA + 1;
    tA = g.reservedTracks + logA / g.sectorsPerTrack;
    idA = g.sectorBaseId + (logA % g.sectorsPerTrack);
    tB = g.reservedTracks + logB / g.sectorsPerTrack;
    idB = g.sectorBaseId + (logB % g.sectorsPerTrack);
}

static bool splitName83(const QString &input, QByteArray &name8,
                        QByteArray &ext3, QString &errOut)
{
    QString base = QFileInfo(input).fileName();
    if (base.isEmpty()) { errOut = "empty filename"; return false; }
    QString up = base.toUpper();
    int dot = up.lastIndexOf('.');
    QString n = (dot >= 0) ? up.left(dot) : up;
    QString e = (dot >= 0) ? up.mid(dot + 1) : QString();
    if (n.isEmpty() || n.size() > 8)
    {
        errOut = QString("invalid name \"%1\" (1..8 chars before dot)").arg(base);
        return false;
    }
    if (e.size() > 3)
    {
        errOut = QString("invalid extension in \"%1\" (0..3 chars)").arg(base);
        return false;
    }
    auto validCh = [](QChar c) {
        return c.isLetterOrNumber() || QString("!#$&@^_{}`~").contains(c);
    };
    for (QChar c : n)
        if (!validCh(c))
        {
            errOut = QString("invalid character in filename \"%1\"").arg(base);
            return false;
        }
    for (QChar c : e)
        if (!validCh(c))
        {
            errOut = QString("invalid character in extension \"%1\"").arg(base);
            return false;
        }
    name8 = n.leftJustified(8, ' ', true).toLatin1();
    ext3 = e.leftJustified(3, ' ', true).toLatin1();
    return true;
}

static QByteArray buildAmsdosHeader(const QByteArray &name8, const QByteArray &ext3,
                                    int loadAddr, int execAddr, int dataLen)
{
    QByteArray h(128, char(0));
    h[0x00] = 0;
    for (int i = 0; i < 8; i++) h[1 + i] = name8[i];
    for (int i = 0; i < 3; i++) h[9 + i] = ext3[i];
    h[0x10] = 0;
    h[0x11] = 0;
    h[0x12] = char(2);
    h[0x13] = char(dataLen & 0xFF);
    h[0x14] = char((dataLen >> 8) & 0xFF);
    h[0x15] = char(loadAddr & 0xFF);
    h[0x16] = char((loadAddr >> 8) & 0xFF);
    h[0x17] = char(0xFF);
    h[0x18] = char(dataLen & 0xFF);
    h[0x19] = char((dataLen >> 8) & 0xFF);
    h[0x1A] = char(execAddr & 0xFF);
    h[0x1B] = char((execAddr >> 8) & 0xFF);
    h[0x40] = char(dataLen & 0xFF);
    h[0x41] = char((dataLen >> 8) & 0xFF);
    h[0x42] = char((dataLen >> 16) & 0xFF);
    unsigned sum = 0;
    for (int i = 0; i < 0x43; i++) sum += static_cast<unsigned char>(h[i]);
    h[0x43] = char(sum & 0xFF);
    h[0x44] = char((sum >> 8) & 0xFF);
    return h;
}

static bool readDirectory(FloppyDrive *drv, const AmsdosGeom &g,
                          QByteArray &dirOut, QString &errOut)
{
    dirOut.resize(g.dirSectors * g.sectorSize);
    for (int i = 0; i < g.dirSectors; i++)
    {
        int track = g.reservedTracks;
        int id = g.sectorBaseId + i;
        int sz = 0;
        BYTE *p = drv->GetSectorDataById(track, 0, id, &sz);
        if (!p || sz < g.sectorSize)
        {
            errOut = QString("cannot read directory sector id &%1 on track %2")
                     .arg(QString::number(id, 16).toUpper()).arg(track);
            return false;
        }
        memcpy(dirOut.data() + i * g.sectorSize, p, g.sectorSize);
    }
    return true;
}

static bool writeDirectory(FloppyDrive *drv, const AmsdosGeom &g,
                           const QByteArray &dir, QString &errOut)
{
    for (int i = 0; i < g.dirSectors; i++)
    {
        int track = g.reservedTracks;
        int id = g.sectorBaseId + i;
        int sz = 0;
        BYTE *p = drv->GetSectorDataById(track, 0, id, &sz);
        if (!p || sz < g.sectorSize)
        {
            errOut = QString("cannot write directory sector id &%1 on track %2")
                     .arg(QString::number(id, 16).toUpper()).arg(track);
            return false;
        }
        memcpy(p, dir.constData() + i * g.sectorSize, g.sectorSize);
    }
    return true;
}

static bool writeBlock(FloppyDrive *drv, const AmsdosGeom &g, int block,
                       const char *data, int len, QString &errOut)
{
    int tA, idA, tB, idB;
    blockToSectors(g, block, tA, idA, tB, idB);
    int szA = 0, szB = 0;
    BYTE *pA = drv->GetSectorDataById(tA, 0, idA, &szA);
    BYTE *pB = drv->GetSectorDataById(tB, 0, idB, &szB);
    if (!pA || szA < g.sectorSize || !pB || szB < g.sectorSize)
    {
        errOut = QString("block %1 maps to missing sector (t%2 id&%3 or t%4 id&%5)")
                 .arg(block).arg(tA).arg(QString::number(idA, 16).toUpper())
                 .arg(tB).arg(QString::number(idB, 16).toUpper());
        return false;
    }
    QByteArray buf(g.blockSize, char(0xE5));
    int n = qMin(len, g.blockSize);
    if (n > 0) memcpy(buf.data(), data, n);
    memcpy(pA, buf.constData(), g.sectorSize);
    memcpy(pB, buf.constData() + g.sectorSize, g.sectorSize);
    return true;
}

static bool amsdosNameEquals(const BYTE *entry, const QByteArray &name8,
                             const QByteArray &ext3)
{
    for (int i = 0; i < 8; i++)
        if ((entry[1 + i] & 0x7F) != static_cast<unsigned char>(name8[i])) return false;
    for (int i = 0; i < 3; i++)
        if ((entry[9 + i] & 0x7F) != static_cast<unsigned char>(ext3[i])) return false;
    return true;
}

static bool writeAmsdosFile(FloppyDrive *drv, const QString &filename,
                            int loadAddr, int execAddr,
                            const QByteArray &bodyBytes,
                            QStringList &log, QString &errOut)
{
    AmsdosGeom g = detectAmsdosGeom(drv);
    if (!g.valid) { errOut = "disc in drive A does not appear to be AMSDOS-formatted (expected DATA or SYSTEM)"; return false; }

    QByteArray name8, ext3;
    if (!splitName83(filename, name8, ext3, errOut)) return false;

    QByteArray header = buildAmsdosHeader(name8, ext3, loadAddr, execAddr, bodyBytes.size());
    QByteArray payload = header + bodyBytes;
    int totalRecords = (payload.size() + 127) / 128;
    int totalBlocks = (payload.size() + g.blockSize - 1) / g.blockSize;
    int extentsNeeded = (totalRecords + 127) / 128;
    if (extentsNeeded < 1) extentsNeeded = 1;

    QByteArray dir;
    if (!readDirectory(drv, g, dir, errOut)) return false;

    BYTE *de = reinterpret_cast<BYTE *>(dir.data());

    for (int i = 0; i < g.dirEntries; i++)
    {
        BYTE *e = de + i * 32;
        if (e[0] <= 15 && amsdosNameEquals(e, name8, ext3))
            e[0] = 0xE5;
    }

    QSet<int> used;
    for (int i = 0; i < g.dirEntries; i++)
    {
        BYTE *e = de + i * 32;
        if (e[0] > 15) continue;
        for (int k = 0; k < 16; k++)
        {
            int b = e[0x10 + k];
            if (b != 0) used.insert(b);
        }
    }

    QVector<int> freeBlocks;
    for (int b = 2; b <= g.maxBlock; b++)
        if (!used.contains(b)) freeBlocks.append(b);

    if (totalBlocks > freeBlocks.size())
    {
        errOut = QString("disc full (%1 blocks needed, %2 free)")
                 .arg(totalBlocks).arg(freeBlocks.size());
        return false;
    }

    QVector<int> freeEntries;
    for (int i = 0; i < g.dirEntries; i++)
        if (de[i * 32] > 15) freeEntries.append(i);

    if (extentsNeeded > freeEntries.size())
    {
        errOut = QString("directory full (%1 entries needed, %2 free)")
                 .arg(extentsNeeded).arg(freeEntries.size());
        return false;
    }

    QVector<int> chosenBlocks = freeBlocks.mid(0, totalBlocks);

    for (int i = 0; i < totalBlocks; i++)
    {
        int blk = chosenBlocks[i];
        int off = i * g.blockSize;
        int remain = payload.size() - off;
        if (!writeBlock(drv, g, blk, payload.constData() + off, remain, errOut))
            return false;
    }

    int recordsRemaining = totalRecords;
    int blockCursor = 0;
    for (int ex = 0; ex < extentsNeeded; ex++)
    {
        int entryIdx = freeEntries[ex];
        BYTE *e = de + entryIdx * 32;
        memset(e, 0, 32);
        e[0] = 0;
        for (int i = 0; i < 8; i++) e[1 + i] = static_cast<unsigned char>(name8[i]);
        for (int i = 0; i < 3; i++) e[9 + i] = static_cast<unsigned char>(ext3[i]);
        e[0x0C] = ex & 0x1F;
        e[0x0D] = 0;
        e[0x0E] = (ex >> 5) & 0x3F;
        int recThis = qMin(128, recordsRemaining);
        e[0x0F] = recThis & 0xFF;
        int blocksThis = (recThis + 7) / 8;
        for (int k = 0; k < 16; k++)
            e[0x10 + k] = (k < blocksThis) ? static_cast<BYTE>(chosenBlocks[blockCursor + k]) : 0;
        blockCursor += blocksThis;
        recordsRemaining -= recThis;
    }

    if (!writeDirectory(drv, g, dir, errOut)) return false;

    log.append(QString("wrote %1 bytes (%2 + 128 header) as %3.%4 to drive A (non-persistent)")
               .arg(payload.size()).arg(bodyBytes.size())
               .arg(QString::fromLatin1(name8).trimmed())
               .arg(QString::fromLatin1(ext3).trimmed()));
    log.append(QString("  load=&%1 exec=&%2 blocks=%3 extents=%4 (%5)")
               .arg(loadAddr, 4, 16, QChar('0')).arg(execAddr, 4, 16, QChar('0'))
               .arg(totalBlocks).arg(extentsNeeded)
               .arg(g.sectorBaseId == 0xC1 ? "DATA" : "SYSTEM"));
    return true;
}

} // namespace

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
    output->viewport()->installEventFilter(this);
    output->viewport()->setCursor(Qt::IBeamCursor);

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

    restoreSession();
    if (tabs->count() == 0) newEditorTab();

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
    saveSession();
    event->accept();
}

void AssemblerWindow::saveSession()
{
    QDir().mkpath(QDir::homePath() + "/.config/cadence");
    QSettings s(QDir::homePath() + "/.config/cadence/settings.cfg", QSettings::IniFormat);
    QStringList paths;
    for (int i = 0; i < tabs->count(); i++)
    {
        QPlainTextEdit *ed = qobject_cast<QPlainTextEdit *>(tabs->widget(i));
        if (!ed) continue;
        QString path = editorFile(ed);
        if (!path.isEmpty()) paths << path;
    }
    s.setValue("assembler/open_files", paths);
    int curIdx = -1;
    if (QPlainTextEdit *cur = currentEditor())
    {
        QString curPath = editorFile(cur);
        curIdx = curPath.isEmpty() ? -1 : paths.indexOf(curPath);
    }
    s.setValue("assembler/current_index", curIdx);
}

void AssemblerWindow::restoreSession()
{
    QSettings s(QDir::homePath() + "/.config/cadence/settings.cfg", QSettings::IniFormat);
    QStringList paths = s.value("assembler/open_files").toStringList();
    int curIdx = s.value("assembler/current_index", -1).toInt();
    for (const QString &path : paths)
    {
        if (!QFileInfo::exists(path)) continue;
        QPlainTextEdit *ed = newEditorTab();
        if (readEditorFromFile(ed, path))
            setEditorFile(ed, path);
        else
        {
            int idx = tabs->indexOf(ed);
            if (idx >= 0) tabs->removeTab(idx);
            ed->deleteLater();
        }
    }
    if (curIdx >= 0 && curIdx < tabs->count())
        tabs->setCurrentIndex(curIdx);
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

    QProgressBar *progressBar = new QProgressBar(statusBar());
    progressBar->setRange(0, 100);
    progressBar->setFixedWidth(540);
    progressBar->setTextVisible(true);
    statusBar()->addPermanentWidget(progressBar);
    progressBar->setValue(0);
    progressBar->setFormat(tr("Lexing..."));
    statusBar()->showMessage(tr("Assembling..."));
    actAssemble->setEnabled(false);
    QElapsedTimer pulseTimer;
    pulseTimer.start();
    QString passLabel[3] = {tr("Lexing"), tr("Pass 1"), tr("Pass 2")};
    Assembler::ProgressFn progress = [&](int pass, int percent, const QString &label) {
        int idx = qBound(0, pass, 2);
        if (label.startsWith(QStringLiteral("READ ")))
            statusBar()->showMessage(tr("Reading %1...").arg(label.mid(5)));
        progressBar->setValue(percent);
        progressBar->setFormat(QString("%1 %2%").arg(passLabel[idx]).arg(percent));
        if (pulseTimer.elapsed() >= 30)
        {
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            pulseTimer.restart();
        }
    };
    AssemblerResult r = assembler.Assemble(ed->toPlainText(), base, progress);
    statusBar()->removeWidget(progressBar);
    progressBar->deleteLater();
    actAssemble->setEnabled(true);
    for (const AssemblerMessage &m : r.messages)
    {
        QString prefix = m.isError ? tr("error") : tr("info");
        QString loc;
        QString navPath;
        if (!m.source.isEmpty())
        {
            loc = QString("(%1:%2) ").arg(QFileInfo(m.source).fileName(), QString::number(m.line));
            navPath = m.source;
        }
        else if (m.line > 0)
        {
            loc = QString("(%1) ").arg(m.line);
            navPath = path;
        }
        appendErrorOutput(QString("%1%2: %3").arg(loc, prefix, m.text), m.isError, navPath, m.line);
    }
    if (!r.ok)
    {
        statusBar()->showMessage(tr("Assembly failed"), 5000);
        return;
    }

    if (EmulatorThread::running)
    {
        if (MainWindow::Instance)
            MainWindow::Instance->suppressNextPauseDebugger = true;
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
    int diskTotal = 0;
    bool anyFailed = false;
    QHash<QString, QByteArray> fileBuffers;
    QStringList fileOrder;
    struct DiskFile { QByteArray bytes; int loadAddr; int execAddr; };
    QHash<QString, DiskFile> diskBuffers;
    QStringList diskOrder;
    for (const AssemblerSegment &s : r.segments)
    {
        if (s.toDisk && !s.fileName.isEmpty())
        {
            if (!diskBuffers.contains(s.fileName))
            {
                DiskFile df;
                df.loadAddr = s.writeOrigin & 0xFFFF;
                df.execAddr = (s.execAddress >= 0) ? (s.execAddress & 0xFFFF) : df.loadAddr;
                diskBuffers.insert(s.fileName, df);
                diskOrder.append(s.fileName);
            }
            diskBuffers[s.fileName].bytes.append(s.bytes);
            diskTotal += s.bytes.size();
            appendOutput(QString("emitted %1 bytes to disc file \"%2\"").arg(s.bytes.size()).arg(s.fileName), false);
            continue;
        }
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
        bool hitLoRom = false, hitUpRom = false, hitRam = false;
        for (int i = 0; i < s.bytes.size(); i++)
        {
            word addr = word((s.writeOrigin + i) & 0xFFFF);
            int bankIdx = addr >> 14;
            int off = addr & 0x3FFF;
            BYTE b = BYTE(uchar(s.bytes[i]));
            BYTE *dst = nullptr;
            enum { HIT_NONE, HIT_LOROM, HIT_UPROM, HIT_RAM } hit = HIT_NONE;
            if (bankIdx == 0 && s.lowerRom == 0)
            {
                dst = CPC::LoROM;
                hit = HIT_LOROM;
            }
            else if (bankIdx == 3 && s.upperRom >= 0)
            {
                if (s.upperRom >= 0x80 && s.upperRom <= 0x9F)
                {
                    if (CPC::cartridgeEnabled && CPC::Cartridge)
                        dst = CPC::Cartridge + (s.upperRom - 0x80) * 0x4000;
                }
                else
                    dst = CPC::HiROMs[s.upperRom];
                hit = HIT_UPROM;
            }
            else
            {
                dst = ramForBank(s.ramBank, bankIdx);
                hit = HIT_RAM;
            }
            if (dst)
            {
                dst[off] = b;
                written++;
                if (hit == HIT_LOROM) hitLoRom = true;
                else if (hit == HIT_UPROM) hitUpRom = true;
                else if (hit == HIT_RAM) hitRam = true;
            }
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
        QStringList parts;
        if (hitLoRom) parts << QStringLiteral("LoROM");
        if (hitUpRom) parts << upperLabel(s.upperRom);
        if (hitRam)   parts << QString("RAM&%1").arg(QString::number(s.ramBank, 16).toUpper());
        QString tgt = parts.isEmpty() ? QStringLiteral("-") : parts.join('+');
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
    if (!diskOrder.isEmpty())
    {
        FloppyDrive *drv = CPC::fdc.GetDrive(0);
        if (!drv || !drv->DiskInserted)
        {
            appendOutput(tr("WRITE DIRECT: no disc inserted in drive A; %1 file(s) not written").arg(diskOrder.size()), true);
            anyFailed = true;
        }
        else
        {
            for (const QString &fname : diskOrder)
            {
                const DiskFile &df = diskBuffers[fname];
                QStringList log;
                QString err;
                if (!writeAmsdosFile(drv, fname, df.loadAddr, df.execAddr, df.bytes, log, err))
                {
                    appendOutput(QString("WRITE DIRECT \"%1\": %2").arg(fname, err), true);
                    anyFailed = true;
                }
                else
                {
                    for (const QString &line : log) appendOutput(line, false);
                }
            }
        }
    }

    QStringList summaryBits;
    if (fileTotal > 0) summaryBits.append(tr("%1 bytes to file(s)").arg(fileTotal));
    if (diskTotal > 0) summaryBits.append(tr("%1 bytes to disc A").arg(diskTotal));
    if (summaryBits.isEmpty())
        appendOutput(tr("Assembled %1 bytes into emulator memory.").arg(total), false);
    else
        appendOutput(tr("Assembled %1 bytes into emulator memory and %2.")
                     .arg(total).arg(summaryBits.join(", ")), false);
    if (anyFailed)
        statusBar()->showMessage(tr("Assembled with skipped bytes"), 5000);
    else
        statusBar()->showMessage(tr("Assembled"), 5000);

    if (MainWindow::Instance)
        MainWindow::Instance->RefreshDebuggerIfOpen();

    EmulatorThread::Run();
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

void AssemblerWindow::appendErrorOutput(const QString &text, bool isError,
                                        const QString &source, int line)
{
    (void)isError;
    output->appendPlainText(text);
    if (!source.isEmpty() && line > 0)
    {
        int blockNum = output->document()->blockCount() - 1;
        outputLocations.insert(blockNum, qMakePair(source, line));
    }
}

void AssemblerWindow::clearOutput()
{
    output->clear();
    outputLocations.clear();
}

void AssemblerWindow::navigateToSource(const QString &path, int line)
{
    QPlainTextEdit *target = nullptr;
    for (int i = 0; i < tabs->count(); i++)
    {
        QPlainTextEdit *e = qobject_cast<QPlainTextEdit *>(tabs->widget(i));
        if (e && editorFile(e) == path)
        {
            tabs->setCurrentIndex(i);
            target = e;
            break;
        }
    }
    if (!target)
    {
        QPlainTextEdit *ed = currentEditor();
        bool reuse = ed && editorFile(ed).isEmpty() &&
                     !ed->document()->isModified() && ed->document()->isEmpty();
        if (!reuse) ed = newEditorTab();
        if (!readEditorFromFile(ed, path)) return;
        setEditorFile(ed, path);
        target = ed;
    }
    if (!target) return;

    QTextBlock block = target->document()->findBlockByNumber(line - 1);
    if (!block.isValid()) block = target->document()->lastBlock();
    QTextCursor cursor(block);
    target->setTextCursor(cursor);
    target->centerCursor();
    target->setFocus();
}

bool AssemblerWindow::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == output->viewport() && ev->type() == QEvent::MouseButtonDblClick)
    {
        QMouseEvent *me = static_cast<QMouseEvent *>(ev);
        QTextCursor cursor = output->cursorForPosition(me->pos());
        int blockNum = cursor.blockNumber();
        auto it = outputLocations.constFind(blockNum);
        if (it != outputLocations.constEnd())
        {
            navigateToSource(it.value().first, it.value().second);
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, ev);
}

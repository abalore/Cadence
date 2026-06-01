#include "MediaController.h"
#include "ZipMedia.h"
#include "CPC.h"
#include "mainwindow.h"
#include <QFileInfo>

MediaController::MediaController(QObject *parent) : QObject(parent) {}

void MediaController::notify(MediaSlot slot, const QString &path, const QString &entry)
{
    QString text;
    if (path.isEmpty())
        text = "<Empty>";
    else if (!entry.isEmpty())
        text = QFileInfo(entry).fileName() + " — " + QFileInfo(path).fileName();
    else
        text = QFileInfo(path).fileName();
    emit mediaChanged(slot, text);
}

QString MediaController::resolveSource(const QString &path, const QString &entry)
{
    if (entry.isEmpty())
        return path;
    if (!tempDir)
        tempDir = std::make_unique<QTemporaryDir>();
    if (!tempDir->isValid())
        return QString();
    return ZipMedia::extractToDir(path, entry, tempDir->path());
}

bool MediaController::LoadDiskA(const QString &path, const QString &entry)
{
    QString src = resolveSource(path, entry);
    if (src.isEmpty()) return false;
    if (!CPC::fdc.GetDrive(0)->InsertDSK((char *)src.toUtf8().data()))
        return false;
    diskAPath = path;
    diskAEntry = entry;
    notify(MediaSlot::DiskA, path, entry);
    return true;
}

bool MediaController::LoadDiskB(const QString &path, const QString &entry)
{
    QString src = resolveSource(path, entry);
    if (src.isEmpty()) return false;
    if (!CPC::fdc.GetDrive(1)->InsertDSK((char *)src.toUtf8().data()))
        return false;
    diskBPath = path;
    diskBEntry = entry;
    notify(MediaSlot::DiskB, path, entry);
    return true;
}

bool MediaController::LoadTape(const QString &path, const QString &entry)
{
    QString src = resolveSource(path, entry);
    if (src.isEmpty()) return false;
    QString extension = src.last(3).toLower();
    if (extension == "wav")
        CPC::tape.LoadWAV((char *)src.toUtf8().data());
    else if (extension == "cdt")
        CPC::tape.LoadCDT((char *)src.toUtf8().data());
    else
        return false;
    tapePath = path;
    tapeEntry = entry;
    notify(MediaSlot::Tape, path, entry);
    return true;
}

bool MediaController::LoadCartridge(const QString &path, const QString &entry)
{
    QString src = resolveSource(path, entry);
    if (src.isEmpty()) return false;
    SaveCartridgeIfDirty();
    CPC::ReadCartridge((char *)src.toUtf8().data());
    cartridgePath = path;
    cartridgeEntry = entry;
    CPC::cartridgeEnabled = true;
    CPC::cartridgeDirty = false;
    notify(MediaSlot::Cartridge, path, entry);
    CPC::Reset();
    return true;
}

void MediaController::InsertBlankCartridge()
{
    CPC::InsertBlankCartridge();
    cartridgePath.clear();
    cartridgeEntry.clear();
    CPC::cartridgeEnabled = true;
    emit mediaChanged(MediaSlot::Cartridge, "<Blank>");
    if (MainWindow::Instance)
        MainWindow::Instance->ResetEmulation();
}

bool MediaController::NewBlankCartridge(const QString &path)
{
    CPC::InsertBlankCartridge();
    CPC::SaveCartridge(path.toUtf8().constData());
    cartridgePath = path;
    cartridgeEntry.clear();
    CPC::cartridgeEnabled = true;
    notify(MediaSlot::Cartridge, path, QString());
    if (MainWindow::Instance)
        MainWindow::Instance->ResetEmulation();
    return true;
}

void MediaController::SaveCartridgeIfDirty()
{
    // Never write back into a zip; the cartridgePath there is the archive.
    if (CPC::cartridgeEnabled && CPC::cartridgeDirty && !cartridgePath.isEmpty()
        && cartridgeEntry.isEmpty())
        CPC::SaveCartridge(cartridgePath.toUtf8().constData());
}

void MediaController::EjectDiskA()
{
    if (CPC::fdc.GetDrive(0)->RemoveDSK())
    {
        diskAPath.clear();
        diskAEntry.clear();
        notify(MediaSlot::DiskA, "", QString());
    }
}

void MediaController::EjectDiskB()
{
    if (CPC::fdc.GetDrive(1)->RemoveDSK())
    {
        diskBPath.clear();
        diskBEntry.clear();
        notify(MediaSlot::DiskB, "", QString());
    }
}

void MediaController::EjectTape()
{
    CPC::tape.Eject();
    tapePath.clear();
    tapeEntry.clear();
    notify(MediaSlot::Tape, "", QString());
}

void MediaController::EjectCartridge()
{
    SaveCartridgeIfDirty();
    CPC::cartridgeDirty = false;
    CPC::cartridgeEnabled = false;
    cartridgePath.clear();
    cartridgeEntry.clear();
    notify(MediaSlot::Cartridge, "", QString());
    CPC::Reset();
}

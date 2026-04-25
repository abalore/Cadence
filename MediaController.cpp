#include "MediaController.h"
#include "CPC.h"
#include "mainwindow.h"
#include <QFileInfo>

MediaController::MediaController(QObject *parent) : QObject(parent) {}

void MediaController::notify(MediaSlot slot, const QString &path)
{
    QString text = path.isEmpty() ? "<Empty>" : QFileInfo(path).fileName();
    emit mediaChanged(slot, text);
}

bool MediaController::LoadDiskA(const QString &path)
{
    if (!CPC::fdc.GetDrive(0)->InsertDSK((char *)path.toUtf8().data()))
        return false;
    diskAPath = path;
    notify(MediaSlot::DiskA, path);
    return true;
}

bool MediaController::LoadDiskB(const QString &path)
{
    if (!CPC::fdc.GetDrive(1)->InsertDSK((char *)path.toUtf8().data()))
        return false;
    diskBPath = path;
    notify(MediaSlot::DiskB, path);
    return true;
}

bool MediaController::LoadTape(const QString &path)
{
    QString extension = path.last(3).toLower();
    if (extension == "wav")
        CPC::tape.LoadWAV((char *)path.toUtf8().data());
    else if (extension == "cdt")
        CPC::tape.LoadCDT((char *)path.toUtf8().data());
    else
        return false;
    tapePath = path;
    notify(MediaSlot::Tape, path);
    return true;
}

bool MediaController::LoadCartridge(const QString &path)
{
    SaveCartridgeIfDirty();
    CPC::ReadCartridge((char *)path.toUtf8().data());
    cartridgePath = path;
    CPC::cartridgeEnabled = true;
    CPC::cartridgeDirty = false;
    notify(MediaSlot::Cartridge, path);
    CPC::Reset();
    return true;
}

void MediaController::InsertBlankCartridge()
{
    CPC::InsertBlankCartridge();
    cartridgePath.clear();
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
    CPC::cartridgeEnabled = true;
    notify(MediaSlot::Cartridge, path);
    if (MainWindow::Instance)
        MainWindow::Instance->ResetEmulation();
    return true;
}

void MediaController::SaveCartridgeIfDirty()
{
    if (CPC::cartridgeEnabled && CPC::cartridgeDirty && !cartridgePath.isEmpty())
        CPC::SaveCartridge(cartridgePath.toUtf8().constData());
}

void MediaController::EjectDiskA()
{
    if (CPC::fdc.GetDrive(0)->RemoveDSK())
    {
        diskAPath.clear();
        notify(MediaSlot::DiskA, "");
    }
}

void MediaController::EjectDiskB()
{
    if (CPC::fdc.GetDrive(1)->RemoveDSK())
    {
        diskBPath.clear();
        notify(MediaSlot::DiskB, "");
    }
}

void MediaController::EjectTape()
{
    CPC::tape.Eject();
    tapePath.clear();
    notify(MediaSlot::Tape, "");
}

void MediaController::EjectCartridge()
{
    SaveCartridgeIfDirty();
    CPC::cartridgeDirty = false;
    CPC::cartridgeEnabled = false;
    cartridgePath.clear();
    notify(MediaSlot::Cartridge, "");
    CPC::Reset();
}

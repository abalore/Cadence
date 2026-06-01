#ifndef MEDIACONTROLLER_H
#define MEDIACONTROLLER_H

#include <QObject>
#include <QString>
#include <QTemporaryDir>
#include <memory>

enum class MediaSlot { Tape, DiskA, DiskB, Cartridge };

class MediaController : public QObject
{
    Q_OBJECT
public:
    explicit MediaController(QObject *parent = nullptr);

    // For each loader, `entry` names the file to pull out of a .zip given in
    // `path`; leave it empty to load `path` directly. Zip-loaded media is
    // extracted to a per-session temp dir and is not written back to the zip.
    bool LoadDiskA(const QString &path, const QString &entry = QString());
    bool LoadDiskB(const QString &path, const QString &entry = QString());
    bool LoadTape(const QString &path, const QString &entry = QString());
    bool LoadCartridge(const QString &path, const QString &entry = QString());
    void InsertBlankCartridge();
    bool NewBlankCartridge(const QString &path);
    void SaveCartridgeIfDirty();

    void EjectDiskA();
    void EjectDiskB();
    void EjectTape();
    void EjectCartridge();

    QString DiskAPath() const { return diskAPath; }
    QString DiskBPath() const { return diskBPath; }
    QString TapePath() const { return tapePath; }
    QString CartridgePath() const { return cartridgePath; }

    // The in-archive entry name for a zip-loaded slot, or empty otherwise.
    QString DiskAEntry() const { return diskAEntry; }
    QString DiskBEntry() const { return diskBEntry; }
    QString TapeEntry() const { return tapeEntry; }
    QString CartridgeEntry() const { return cartridgeEntry; }

signals:
    void mediaChanged(MediaSlot slot, const QString &displayText);

private:
    QString diskAPath;
    QString diskBPath;
    QString tapePath;
    QString cartridgePath;

    QString diskAEntry;
    QString diskBEntry;
    QString tapeEntry;
    QString cartridgeEntry;

    std::unique_ptr<QTemporaryDir> tempDir;

    // If entry is non-empty, extract it from the zip at `path` into the
    // session temp dir and return the extracted path; otherwise return `path`.
    // Returns empty on extraction failure.
    QString resolveSource(const QString &path, const QString &entry);

    void notify(MediaSlot slot, const QString &path, const QString &entry);
};

#endif // MEDIACONTROLLER_H

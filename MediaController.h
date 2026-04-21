#ifndef MEDIACONTROLLER_H
#define MEDIACONTROLLER_H

#include <QObject>
#include <QString>

enum class MediaSlot { Tape, DiskA, DiskB, Cartridge };

class MediaController : public QObject
{
    Q_OBJECT
public:
    explicit MediaController(QObject *parent = nullptr);

    bool LoadDiskA(const QString &path);
    bool LoadDiskB(const QString &path);
    bool LoadTape(const QString &path);
    bool LoadCartridge(const QString &path);

    void EjectDiskA();
    void EjectDiskB();
    void EjectTape();
    void EjectCartridge();

    QString DiskAPath() const { return diskAPath; }
    QString DiskBPath() const { return diskBPath; }
    QString TapePath() const { return tapePath; }
    QString CartridgePath() const { return cartridgePath; }

signals:
    void mediaChanged(MediaSlot slot, const QString &displayText);

private:
    QString diskAPath;
    QString diskBPath;
    QString tapePath;
    QString cartridgePath;

    void notify(MediaSlot slot, const QString &path);
};

#endif // MEDIACONTROLLER_H

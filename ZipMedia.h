#ifndef ZIPMEDIA_H
#define ZIPMEDIA_H

#include <QString>
#include <QStringList>

// Thin wrapper around miniz for reading media out of .zip archives.
namespace ZipMedia
{
    // True if path has a .zip extension.
    bool isZip(const QString &path);

    // Names of the entries in zipPath whose filename ends with one of exts
    // (case-insensitive, e.g. {"dsk"}). Directories are skipped. Returns the
    // full in-archive names (which may include a subdirectory prefix).
    QStringList listEntries(const QString &zipPath, const QStringList &exts);

    // Extract entry from zipPath into destDir, returning the full path of the
    // written file, or an empty string on failure. The on-disk name is the
    // entry's base name.
    QString extractToDir(const QString &zipPath, const QString &entry,
                         const QString &destDir);
}

#endif // ZIPMEDIA_H

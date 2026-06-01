#include "ZipMedia.h"
#include "miniz.h"
#include <QFileInfo>
#include <QDir>

namespace ZipMedia
{

bool isZip(const QString &path)
{
    return path.endsWith(".zip", Qt::CaseInsensitive);
}

static bool matchesExt(const QString &name, const QStringList &exts)
{
    for (const QString &e : exts)
        if (name.endsWith("." + e, Qt::CaseInsensitive))
            return true;
    return false;
}

QStringList listEntries(const QString &zipPath, const QStringList &exts)
{
    QStringList out;
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));
    if (!mz_zip_reader_init_file(&zip, zipPath.toUtf8().constData(), 0))
        return out;

    mz_uint n = mz_zip_reader_get_num_files(&zip);
    for (mz_uint i = 0; i < n; i++)
    {
        if (mz_zip_reader_is_file_a_directory(&zip, i))
            continue;
        mz_zip_archive_file_stat st;
        if (!mz_zip_reader_file_stat(&zip, i, &st))
            continue;
        QString name = QString::fromUtf8(st.m_filename);
        if (matchesExt(name, exts))
            out << name;
    }
    mz_zip_reader_end(&zip);
    return out;
}

QString extractToDir(const QString &zipPath, const QString &entry,
                     const QString &destDir)
{
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));
    if (!mz_zip_reader_init_file(&zip, zipPath.toUtf8().constData(), 0))
        return QString();

    int idx = mz_zip_reader_locate_file(&zip, entry.toUtf8().constData(),
                                        nullptr, 0);
    QString result;
    if (idx >= 0)
    {
        // Flatten any in-archive subdirectory to a plain base name.
        QString base = QFileInfo(entry).fileName();
        QString dst  = QDir(destDir).filePath(base);
        if (mz_zip_reader_extract_to_file(&zip, (mz_uint)idx,
                                          dst.toUtf8().constData(), 0))
            result = dst;
    }
    mz_zip_reader_end(&zip);
    return result;
}

} // namespace ZipMedia

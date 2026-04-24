#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <QByteArray>
#include <QString>
#include <QVector>
#include <functional>

struct AssemblerMessage
{
    int line;
    QString text;
    bool isError;
    QString source;
};

struct AssemblerSegment
{
    int origin = 0;       // logical address (labels, instruction encoding)
    int writeOrigin = 0;  // physical output address (where bytes are placed)
    int lowerRom = -1;   // -1 disabled (RAM), 0 enabled (Lower ROM)
    int upperRom = -1;   // -1 disabled (RAM), 0..15 Upper ROM slot
    int ramBank  = 0xC0; // CPC RAM bank select (#C0..#FF, we support #C0..#C7)
    QString fileName;    // if non-empty, bytes go to this file instead of memory
    bool toDisk = false; // if true, fileName targets the DSK in drive A (AMSDOS)
    int execAddress = -1;// AMSDOS exec address (-1 = use load address)
    QByteArray bytes;
};

struct AssemblerResult
{
    bool ok = false;
    QVector<AssemblerMessage> messages;
    QVector<AssemblerSegment> segments;
};

class Assembler
{
public:
    using ProgressFn = std::function<void(int /*pass*/, int /*percent*/, const QString &/*label*/)>;
    AssemblerResult Assemble(const QString &source, const QString &basePath = QString(),
                             const ProgressFn &progress = ProgressFn());
};

#endif // ASSEMBLER_H

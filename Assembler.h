#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <QByteArray>
#include <QHash>
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

struct AssemblerSectorRef
{
    int drive = 0;   // 0 = A, 1 = B
    int track = 0;   // decimal
    int sector = 0;  // sector ID (CPC convention, e.g. 0xC1..)
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
    QVector<AssemblerSectorRef> sectors; // if non-empty, bytes go to these DSK sectors in order
    QByteArray bytes;
};

struct AssemblerSaveRegion
{
    int address = 0;
    int size = 0;
};

struct AssemblerSaveRequest
{
    QString filename;
    bool toDisk = false;
    QVector<AssemblerSaveRegion> regions;
    int execAddress = -1;
};

struct AssemblerRunRequest
{
    bool valid = false;
    int address = 0;
    int breakpoint = -1;
};

struct AssemblerResult
{
    bool ok = false;
    QVector<AssemblerMessage> messages;
    QVector<AssemblerSegment> segments;
    QVector<AssemblerSaveRequest> saves;
    AssemblerRunRequest run;
    QHash<QString, int> symbols;
};

class Assembler
{
public:
    using ProgressFn = std::function<void(int /*pass*/, int /*percent*/, const QString &/*label*/)>;
    // MemoryReadFn returns one byte from emulator memory at `address`, respecting the
    // current WRITE-DIRECT mapping (lowerRom/upperRom/ramBank).
    using MemoryReadFn = std::function<int(int address, int lowerRom, int upperRom, int ramBank)>;
    AssemblerResult Assemble(const QString &source, const QString &basePath = QString(),
                             const ProgressFn &progress = ProgressFn(),
                             const MemoryReadFn &memReader = MemoryReadFn());
};

#endif // ASSEMBLER_H

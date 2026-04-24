#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <QByteArray>
#include <QString>
#include <QVector>

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
    AssemblerResult Assemble(const QString &source, const QString &basePath = QString());
};

#endif // ASSEMBLER_H

#include "FloppyDrive.h"
#include <stdio.h>
#include <stdlib.h>
#include <QFileInfo>

using namespace std;

bool FloppyDrive::InsertDSK(char *fn)
{
    SaveIfDirty();
    FreeBuffer();
    DiskInserted = false;
    filename.clear();
    qint64 sz = QFileInfo(fn).size();
    if (sz <= 0) return false;
    FILE *f = fopen(fn, "rb");
    if (f)
    {
        buffer = (BYTE *)malloc(sz);
        bufferSize = sz;
        size_t n = fread(buffer, 1, bufferSize, f);
        fclose(f);
        if (n == bufferSize && dsk.Init(buffer, bufferSize))
        {
            DiskInserted = true;
            filename = fn;
        }
        else
            FreeBuffer();
    }
    return DiskInserted;
}

bool FloppyDrive::RemoveDSK()
{
    if (DiskInserted)
    {
        SaveIfDirty();
        DiskInserted = false;
        filename.clear();
        FreeBuffer();
        return true;
    }
    return false;
}

void FloppyDrive::FreeBuffer()
{
    if (buffer != 0)
    {
        free(buffer);
        buffer = 0;
    }
}

void FloppyDrive::SaveIfDirty()
{
    if (!dirty || buffer == nullptr || filename.empty()) return;
    FILE *f = fopen(filename.c_str(), "wb");
    if (f)
    {
        size_t n = fwrite(buffer, 1, bufferSize, f);
        (void)n;
        fclose(f);
    }
    dirty = false;
}

SectorInfo FloppyDrive::GetSectorInfo(BYTE track, BYTE side, BYTE sector)
{
    SectorInfo si;
    if (DiskInserted)
        si = dsk.GetSectorInfo(track, side, sector);
    else
        si.SI_ID = 0xFF;
    return si;
}

BYTE FloppyDrive::GetSectorID(BYTE track, BYTE side)
{
    if (DiskInserted)
        return dsk.GetSectorID(track, side);
    else
        return 0x00;
}

BYTE FloppyDrive::GetSides()
{
    return DiskInserted ? dsk.sides : 1;
}

bool FloppyDrive::FormatTrack(int track, int side, BYTE sizeCode, BYTE sectorCount, BYTE filler, const BYTE *sectorHeaders)
{
    if (!DiskInserted) return false;
    if (dsk.FormatTrack(track, side, sizeCode, sectorCount, filler, sectorHeaders))
    {
        dirty = true;
        return true;
    }
    return false;
}

bool FloppyDrive::SetSectorMark(BYTE track, BYTE side, BYTE sectorId, bool deleted)
{
    if (!DiskInserted) return false;
    if (dsk.SetSectorMark(track, side, sectorId, deleted))
    {
        dirty = true;
        return true;
    }
    return false;
}

SectorInfo FloppyDrive::GetPhysicalSectorInfo(BYTE track, BYTE side, BYTE position)
{
    SectorInfo si;
    if (DiskInserted)
        si = dsk.GetPhysicalSectorInfo(track, side, position);
    else
        si.SI_ID = 0xFF;
    return si;
}

BYTE *FloppyDrive::GetSectorDataById(BYTE track, BYTE side, BYTE sectorId, int *sizeOut)
{
    if (!DiskInserted) return nullptr;
    for (BYTE pos = 0; pos < 64; pos++)
    {
        SectorInfo si = dsk.GetPhysicalSectorInfo(track, side, pos);
        if (si.SI_ID == 0xFE || si.SI_ID == 0xFF) break;
        if (si.SI_ID == sectorId)
        {
            if (sizeOut) *sizeOut = 128 << (si.SI_size & 0x07);
            return si.SectorData[0];
        }
    }
    return nullptr;
}

BYTE FloppyDrive::GetTracks() const
{
    return DiskInserted ? dsk.tracks : 0;
}

BYTE FloppyDrive::GetSidesCount() const
{
    return DiskInserted ? dsk.sides : 0;
}

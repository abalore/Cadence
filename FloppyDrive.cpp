#include "FloppyDrive.h"
#include <stdio.h>
#include <stdlib.h>
#include <filesystem>

using namespace std;

bool FloppyDrive::InsertDSK(char *filename)
{
    FreeBuffer();
    DiskInserted = false;
    bufferSize = filesystem::file_size(filename);
    FILE *f = fopen(filename, "rb");
    if (f)
    {
        buffer = (BYTE *)malloc(bufferSize);
        fread(buffer, 1, bufferSize, f);
        fclose(f);
        if (dsk.Init(buffer, bufferSize))
            DiskInserted = true;
        else
            FreeBuffer();
    }
    return DiskInserted;
}

bool FloppyDrive::RemoveDSK()
{
    if (DiskInserted)
    {
        DiskInserted = false;
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

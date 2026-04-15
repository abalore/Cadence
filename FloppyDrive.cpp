#include "FloppyDrive.h"
#include <stdio.h>
#include <stdlib.h>
#include <filesystem>

using namespace std;

bool FloppyDrive::InsertDSK(char *filename)
{
    FreeBuffer();
    bufferSize = filesystem::file_size(filename);
    buffer = (BYTE *)malloc(bufferSize);
    FILE *f = fopen(filename, "r");
    fread(buffer, 1, bufferSize, f);
    fclose(f);
    if (dsk.Init(buffer))
        DiskInserted = true;
    else
    {
        DiskInserted = false;
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

SectorInfo FloppyDrive::GetSectorInfo(BYTE track, BYTE sector)
{
    SectorInfo si;
    if (DiskInserted)
    {
        si = dsk.GetSectorInfo(track, sector);
        if (si.SectorData == NULL)
            si.SI_C = 0xFE;
    }
    else
        si.SI_C = 0xFF;
    return si;
}

BYTE FloppyDrive::GetSectorID(BYTE track)
{
    if (DiskInserted)
    {
        return dsk.GetSectorID(track);
    }
    else return 0x00;
}

#include "Headers/FloppyDrive.h"
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

BYTE *FloppyDrive::GetSectorData(BYTE track, BYTE sector)
{
    if (DiskInserted)
    {
        return dsk.GetSectorData(track, sector);
    }
    else return nullptr;
}

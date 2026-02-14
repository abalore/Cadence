#ifndef FLOPPYDRIVE_H
#define FLOPPYDRIVE_H

#include "defs.h"
#include "DSK.h"

class FloppyDrive
{
public:
    bool InsertDSK(char *filename);
    bool RemoveDSK();
    BYTE *GetSectorData(BYTE track, BYTE sector);
    bool DiskInserted;
private:
    void FreeBuffer();
    BYTE *buffer = 0;
    unsigned long bufferSize;
    class DSK dsk;
};

#endif // FLOPPYDRIVE_H

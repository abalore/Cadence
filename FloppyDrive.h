#ifndef FLOPPYDRIVE_H
#define FLOPPYDRIVE_H

#include "defs.h"
#include "DSK.h"

class FloppyDrive
{
public:
    ~FloppyDrive() { FreeBuffer(); }
    bool InsertDSK(char *filename);
    bool RemoveDSK();
    SectorInfo GetSectorInfo(BYTE track, BYTE side, BYTE sector);
    BYTE GetSectorID(BYTE track, BYTE side);
    BYTE GetSides();
    bool DiskInserted;
private:
    void FreeBuffer();
    BYTE *buffer = 0;
    unsigned long bufferSize;
    class DSK dsk;
};

#endif // FLOPPYDRIVE_H

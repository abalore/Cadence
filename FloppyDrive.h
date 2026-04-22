#ifndef FLOPPYDRIVE_H
#define FLOPPYDRIVE_H

#include "defs.h"
#include "DSK.h"
#include <string>

class FloppyDrive
{
public:
    ~FloppyDrive() { SaveIfDirty(); FreeBuffer(); }
    bool InsertDSK(char *filename);
    bool RemoveDSK();
    SectorInfo GetSectorInfo(BYTE track, BYTE side, BYTE sector);
    BYTE GetSectorID(BYTE track, BYTE side);
    BYTE GetSides();
    void MarkDirty() { dirty = true; }
    void SetWriteProtect(bool wp) { writeProtect = wp; }
    bool IsWriteProtected() const { return writeProtect; }
    bool FormatTrack(int track, int side, BYTE sizeCode, BYTE sectorCount, BYTE filler, const BYTE *sectorHeaders);
    bool SetSectorMark(BYTE track, BYTE side, BYTE sectorId, bool deleted);
    SectorInfo GetPhysicalSectorInfo(BYTE track, BYTE side, BYTE position);
    bool DiskInserted;
private:
    void FreeBuffer();
    void SaveIfDirty();
    BYTE *buffer = 0;
    unsigned long bufferSize;
    std::string filename;
    bool dirty = false;
    bool writeProtect = false;
    class DSK dsk;
};

#endif // FLOPPYDRIVE_H

#ifndef DSK_H
#define DSK_H

#include "defs.h"
#include <vector>
#include <optional>

struct TrackInfo
{
public:
    int SectorSize;
    int NumberOfSectors;
    BYTE *SectorInfo;
    BYTE *SectorData;
};

struct SectorInfo
{
public:
    BYTE SI_C;
    BYTE SI_H;
    BYTE SI_ID;
    BYTE SI_size;
    BYTE SI_reg1;
    BYTE SI_reg2;
    BYTE copies;
    BYTE *SectorData[3];
};

class DSK
{
public:
    bool Init(BYTE *dskFileData, size_t dataSize);
    TrackInfo GetTrackInfo(int track, int side);
    SectorInfo GetSectorInfo(BYTE track, BYTE side, BYTE sector);
    BYTE GetSectorID(BYTE track, BYTE side);
    bool FormatTrack(int track, int side, BYTE sizeCode, BYTE sectorCount, BYTE filler, const BYTE *sectorHeaders);
    BYTE *data;
    size_t dataSize;
    BYTE tracks;
    BYTE sides;
    word trackSize;
    bool isExtended;
private:
    void AddSector(int track, int side, int sector, BYTE *info, BYTE *address);
    std::vector<std::vector<std::vector<std::optional<SectorInfo>>>> sectors;
    void ParseSectors(bool extended);
};

#endif // DSK_H

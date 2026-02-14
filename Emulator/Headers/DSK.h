#ifndef DSK_H
#define DSK_H

#include "defs.h"

struct TrackInfo
{
public:
    int SectorSize;
    int NumberOfSectors;
};

struct SectorInfo
{
public:
    int ID;
    int Size;
};

class DSK
{
public:
    bool Init(BYTE *dskFileData);
    TrackInfo GetTrackInfo(int track, int side);
    BYTE *GetSectorData(BYTE track, BYTE sector);
    BYTE *data;
    BYTE tracks;
    BYTE sides;
    word trackSize;
    bool isExtended;
};

#endif // DSK_H

#ifndef DSK_H
#define DSK_H

#include "defs.h"

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
    BYTE *SectorData;
};

class DSK
{
public:
    bool Init(BYTE *dskFileData);
    TrackInfo GetTrackInfo(int track, int side);
    SectorInfo GetSectorInfo(BYTE track, BYTE sector);
    BYTE GetSectorID(BYTE track);
    BYTE *data;
    BYTE tracks;
    BYTE sides;
    word trackSize;
    bool isExtended;
private:
    SectorInfo sectors[100][20];
    void LoadNormalDSK();
    void LoadExtendedDSK();
};

#endif // DSK_H

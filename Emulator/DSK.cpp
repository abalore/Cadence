#include "Headers/DSK.h"

#include <string.h>

bool DSK::Init(BYTE *dskFileData)
{
    data = dskFileData;
    tracks = dskFileData[0x30];
    sides = dskFileData[0x31];
    trackSize = dskFileData[0x32] + dskFileData[0x33] * 256;
    if (strncmp("MV - CPC", (const char *)dskFileData, 8) == 0)
    {
        isExtended = false;
        return true;
    }
    else if (strncmp("EXTENDED", (const char *)dskFileData, 8) == 0)
    {
        isExtended = true;
        return true;
    }
    return false;
}

TrackInfo DSK::GetTrackInfo(int track, int side)
{
    int offset = 0;
    if (isExtended)
    {
        for (int i = 0; i < track * sides + side; i++)
            offset += data[0x0034] * 256 + 0x100;
    }
    else
        offset = trackSize * ((track - 1) * sides + side) + 0x0100;
    return TrackInfo{ data[offset + 0x14], data[offset + 0x15], data + 0x0100 };
}

SectorInfo *DSK::GetSectorInfo(BYTE track, BYTE sector)
{
    TrackInfo trackInfo = GetTrackInfo(track, 0);
    for (int i = 0; i < trackInfo.NumberOfSectors; i++)
    {
        SectorInfo *si = (SectorInfo *) trackInfo.SectorInfo + i * 8;
        if (si->SI_ID == sector)
            return si;
    }
    return nullptr;
}

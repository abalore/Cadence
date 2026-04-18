#include "DSK.h"

#include <string.h>

bool DSK::Init(BYTE *dskFileData, size_t dataSize)
{
    sectors.clear();
    data = dskFileData;
    this->dataSize = dataSize;
    tracks = dskFileData[0x30];
    sides = dskFileData[0x31];
    sectors.resize(tracks);
    if (strncmp("MV - CPC", (const char *)dskFileData, 8) == 0)
    {
        trackSize = dskFileData[0x32] + dskFileData[0x33] * 256;
        ParseSectors(false);
        return true;
    }
    else if (strncmp("EXTENDED", (const char *)dskFileData, 8) == 0)
    {
        ParseSectors(true);
        return true;
    }
    return false;
}

void DSK::AddSector(int track, int sector, BYTE *info, BYTE *address)
{
    auto& trackSectors = sectors[track];
    if (sector >= (int)trackSectors.size())
        trackSectors.resize(sector + 1);

    if (!trackSectors[sector].has_value())
    {
        trackSectors[sector].emplace(SectorInfo
            {
                info[0],
                info[1],
                info[2],
                info[3],
                info[4],
                info[5],
                1,
                {address, NULL, NULL}
            });
    }
    else
    {
        SectorInfo& si = trackSectors[sector].value();
        if (si.copies == 3)
            return;
        si.SectorData[si.copies] = address;
        si.copies++;
    }
}

void DSK::ParseSectors(bool extended)
{
    int t = 0;
    isExtended = extended;
    BYTE *trackInfo = data + 0x100;
    while (t < tracks)
    {
        if (trackInfo >= data + dataSize) break;
        if (isExtended) trackSize = data[0x34 + t] * 0x0100;
        if (trackInfo[0] == 'T')
        {
            int s = 0;
            int sz = 0;
            BYTE ns = trackInfo[0x15];
            if (trackSize > 0)
                while (s < ns)
                {
                    BYTE *sectorInfo = trackInfo + 0x18 + s * 0x08;
                    BYTE sectorNum = (sectorInfo[2] & 0x3F) - 1;
                    int realSize = isExtended ? sectorInfo[6] + sectorInfo[7] * 0x0100 : (1 << trackInfo[0x14]) * 0x0080;
                    AddSector(t, sectorNum, sectorInfo, trackInfo + 0x0100 + sz);
                    sz += realSize;
                    s++;
                }
            trackInfo += trackSize;
            t++;
        } else break;
    }
}

SectorInfo DSK::GetSectorInfo(BYTE track, BYTE sector)
{
    BYTE idx = (sector & 0x3F) - 1;
    if (track >= sectors.size() || idx >= sectors[track].size() || !sectors[track][idx].has_value())
        return SectorInfo { track, 0, 0xFF, 0, 0, 0, 0, {NULL, NULL, NULL}};
    return sectors[track][idx].value();
}

BYTE DSK::GetSectorID(BYTE track)
{
    if (track >= sectors.size() || sectors[track].empty() || !sectors[track][0].has_value())
        return 0xFF;
    return sectors[track][0].value().SI_ID;
}

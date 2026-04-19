#include "DSK.h"

#include <string.h>

bool DSK::Init(BYTE *dskFileData, size_t dataSize)
{
    sectors.clear();
    data = dskFileData;
    this->dataSize = dataSize;
    tracks = dskFileData[0x30];
    sides = dskFileData[0x31];
    if (sides < 1) sides = 1;
    sectors.resize(tracks, std::vector<std::vector<std::optional<SectorInfo>>>(sides));
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

void DSK::AddSector(int track, int side, int sector, BYTE *info, BYTE *address)
{
    if (track >= (int)sectors.size() || side >= (int)sectors[track].size())
        return;
    auto& trackSectors = sectors[track][side];
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
    isExtended = extended;
    BYTE *trackInfo = data + 0x100;
    int totalBlocks = tracks * sides;
    for (int b = 0; b < totalBlocks; b++)
    {
        if (trackInfo >= data + dataSize) break;
        if (isExtended) trackSize = data[0x34 + b] * 0x0100;
        if (trackSize == 0) { trackInfo += trackSize; continue; }
        if (trackInfo[0] != 'T') break;

        int logicalTrack = b / sides;
        int logicalSide  = b % sides;
        int s = 0, sz = 0;
        BYTE ns = trackInfo[0x15];
        while (s < ns)
        {
            BYTE *sectorInfo = trackInfo + 0x18 + s * 0x08;
            BYTE sectorNum = (sectorInfo[2] & 0x3F) - 1;
            int realSize = isExtended ? sectorInfo[6] + sectorInfo[7] * 0x0100 : (1 << trackInfo[0x14]) * 0x0080;
            AddSector(logicalTrack, logicalSide, sectorNum, sectorInfo, trackInfo + 0x0100 + sz);
            sz += realSize;
            s++;
        }
        trackInfo += trackSize;
    }
}

SectorInfo DSK::GetSectorInfo(BYTE track, BYTE side, BYTE sector)
{
    BYTE idx = (sector & 0x3F) - 1;
    if (track >= sectors.size() || side >= sectors[track].size()
        || idx >= sectors[track][side].size() || !sectors[track][side][idx].has_value())
        return SectorInfo { track, side, 0xFF, 0, 0, 0, 0, {NULL, NULL, NULL}};
    return sectors[track][side][idx].value();
}

BYTE DSK::GetSectorID(BYTE track, BYTE side)
{
    if (track >= sectors.size() || side >= sectors[track].size()
        || sectors[track][side].empty() || !sectors[track][side][0].has_value())
        return 0xFF;
    return sectors[track][side][0].value().SI_ID;
}

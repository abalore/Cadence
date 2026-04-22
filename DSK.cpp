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

bool DSK::FormatTrack(int track, int side, BYTE sizeCode, BYTE sectorCount, BYTE filler, const BYTE *hdrs)
{
    if (track < 0 || side < 0 || track >= tracks || side >= sides) return false;
    if (sectorCount == 0 || sectorCount > 29) return false;

    int blockIdx = track * sides + side;
    int sectorSize = (1 << sizeCode) * 128;
    int needed = 0x100 + sectorCount * sectorSize;

    BYTE *tp;
    int avail;
    if (isExtended)
    {
        int off = 0x100;
        for (int i = 0; i < blockIdx; i++)
            off += data[0x34 + i] * 0x100;
        tp = data + off;
        avail = data[0x34 + blockIdx] * 0x100;
    }
    else
    {
        tp = data + 0x100 + blockIdx * trackSize;
        avail = trackSize;
    }
    if (needed > avail) return false;

    memset(tp, 0, avail);
    memcpy(tp, "Track-Info\r\n", 12);
    tp[0x10] = (BYTE)track;
    tp[0x11] = (BYTE)side;
    tp[0x14] = sizeCode;
    tp[0x15] = sectorCount;
    tp[0x16] = 0x4E;
    tp[0x17] = filler;

    for (int i = 0; i < sectorCount; i++)
    {
        BYTE *si = tp + 0x18 + i * 8;
        si[0] = hdrs[i * 4 + 0];
        si[1] = hdrs[i * 4 + 1];
        si[2] = hdrs[i * 4 + 2];
        si[3] = hdrs[i * 4 + 3];
        si[4] = 0;
        si[5] = 0;
        if (isExtended)
        {
            si[6] = (BYTE)(sectorSize & 0xFF);
            si[7] = (BYTE)((sectorSize >> 8) & 0xFF);
        }
        memset(tp + 0x100 + i * sectorSize, filler, sectorSize);
    }

    sectors[track][side].clear();
    for (int i = 0; i < sectorCount; i++)
    {
        BYTE *si = tp + 0x18 + i * 8;
        AddSector(track, side, (si[2] & 0x3F) - 1, si, tp + 0x100 + i * sectorSize);
    }
    return true;
}

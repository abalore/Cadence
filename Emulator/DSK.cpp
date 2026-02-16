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
        LoadNormalDSK();
        return true;
    }
    else if (strncmp("EXTENDED", (const char *)dskFileData, 8) == 0)
    {
        LoadExtendedDSK();
        return true;
    }
    return false;
}

void DSK::LoadNormalDSK()
{
    int t = 0;
    isExtended = false;
    while (t < tracks)
    {
        int s = 0;
        BYTE *trackInfo = (data + 0x100 + t * trackSize);
        int sz = trackInfo[0x14] * 0x0100;
        BYTE ns = trackInfo[0x15];
        while (s < ns)
        {
            BYTE *sectorInfo = trackInfo + 0x18 + s * 0x08;
            BYTE sectorNum = (sectorInfo[2] & 0x3F) - 1;
            sectors[t][sectorNum] = SectorInfo
                {
                    sectorInfo[0],
                    sectorInfo[1],
                    sectorInfo[2],
                    sectorInfo[3],
                    sectorInfo[4],
                    sectorInfo[5],
                    trackInfo + 0x0100 + sz * s
                };
            s++;
        }
        t++;
    }
}

void DSK::LoadExtendedDSK()
{
    int t = 0;
    isExtended = true;
    BYTE *trackInfo = data + 0x0100;
    while (t < tracks)
    {
        if (trackInfo[0] == 'T')
        {
            BYTE ns = trackInfo[0x15];
            int s = 0;
            int sz = 0;
            int trackSize = data[0x34 + t] * 0x0100;
            if (trackSize > 0)
            {
                while (s < ns)
                {
                    BYTE *sectorInfo = trackInfo + 0x18 + s * 0x08;
                    BYTE sectorNum = (sectorInfo[2] & 0x3F) - 1;
                    sectors[t][sectorNum] = SectorInfo
                        {
                            sectorInfo[0],
                            sectorInfo[1],
                            sectorInfo[2],
                            sectorInfo[3],
                            sectorInfo[4],
                            sectorInfo[5],
                            trackInfo + 0x0100 + sz
                        };
                    sz += sectorInfo[6] + sectorInfo[7] * 0x0100;
                    s++;
                }
            }
            trackInfo += trackSize;
            t++;
        } else break;
    }
}

SectorInfo DSK::GetSectorInfo(BYTE track, BYTE sector)
{
    return sectors[track][(sector & 0x3F) - 1];
}

BYTE DSK::GetSectorID(BYTE track)
{
    return sectors[track][0].SI_ID;
}

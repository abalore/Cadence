#include "Headers/DSK.h"

#include <string.h>

bool DSK::Init(BYTE *dskFileData, unsigned int size)
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
        offset = trackSize * (track * sides + side) + 0x0100;
    return TrackInfo{data[offset + 0x14], data[offset + 0x15]};
}

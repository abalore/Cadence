#ifndef CDT_H
#define CDT_H

#include "defs.h"

enum STATE
{
    GETID,
    READPARAM,
    PILOT,
    SYNC1,
    SYNC2,
    TDATA,
    POSTPILOT,
    PAUSE,
    SYNCLAST,
    NEWPARAM,
    NAME,
    NAME00,
    GAP
};

enum DATASTATE
{
    INIT,
    STARTBITS,
    DATABITS,
    PARITYBIT,
    STOPBITS,
    SECURITYBITSSTART,
    SECURITYBITSFETCH,
    SECURITYBITSPROCESS,
    SECURITYBITSEND,
    FINISHED
};

#define PILOTLENGTH 619
#define SYNCFIRST 191
#define SYNCSECOND 210
#define ZEROPULSE 244
#define ONEPULSE 489
#define PILOTNUMBERL 8063
#define PILOTNUMBERH 3223
#define PAUSELENGTH 2000

class CDT
{
public:
    bool Init(BYTE *cdtFileData, unsigned int size);
    bool GetNextLevel();
    bool EndOfFile;
private:
    bool ReadByte();
    bool ReadWord();
    bool ReadLong();
    bool ReadDword();
    void Process();
    void ProcessParams();
    void ProcessData();
    void WriteData();
    word TicksToMicros(word ticks);
    STATE state;
    STATE stateAfterPilot;
    STATE stateAfterPause;
    DATASTATE dataState;
    BYTE currentByte;
    word zeroPulse;
    word onePulse;
    word pauseLength;
    word pilotLength;
    word pilotPulses;
    BYTE blockType;
    word sync1Length;
    word sync2Length = 0;
    dword loopStart = 0;
    word loopCount = 0;
    BYTE usedBitsInLastByte = 8;
    BYTE outByte;
    word outWord;
    dword outDword;
    dword bytesRead;
    word bytesToRead;
    dword currentPeriod = 0;
    BYTE *data;
    unsigned int dataSize;
    BYTE currentBit;
    BYTE pass;
    bool level;
};

#endif // CDT_H

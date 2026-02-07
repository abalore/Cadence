#include "CDT.h"
#include <string.h>

bool CDT::Init(BYTE *cdtFileData, unsigned int size)
{
    if (strncmp("ZXTape!", (const char *)cdtFileData, 7) == 0)
    {
        bytesRead = 10;
        data = cdtFileData;
        dataSize = size;
        EndOfFile = false;
        currentPeriod = 0;
        level = false;
        state = STATE::PAUSE;
        stateAfterPause = STATE::GETID;
        pauseLength = PAUSELENGTH;
        return true;
    }
    return false;
}

bool CDT::GetNextLevel()
{
    if (currentPeriod > 0)
    {
        currentPeriod--;
        return level;
    }
    while (!EndOfFile)
    {
        Process();
        if (currentPeriod > 0)
            currentPeriod /= 23;
        if (currentPeriod > 0)
        {
            level = !level;
            return level;
        }
    }
    return false;
}

bool CDT::ReadByte()
{
    outByte = data[bytesRead++];
    return bytesRead < dataSize;
}

bool CDT::ReadWord()
{
    if (ReadByte())
    {
        outWord = outByte;
        if (ReadByte())
        {
            outWord += (word)outByte * 0x100;
            return true;
        }
        else
            return false;
    }
    else
        return false;
}

bool CDT::ReadLong()
{
    if (ReadWord())
    {
        outDword = (dword) outWord;
        if (ReadByte())
        {
            outDword += (dword)outByte * 0x10000;
            return true;
        }
        else
            return false;
    }
    else
        return false;
}

bool CDT::ReadDword() {
    if (ReadLong())
    {
        if (ReadByte())
        {
            outDword += (dword)outByte * 0x1000000;
            return true;
        }
        else
            return false;
    }
    return false;
}

void CDT::Process()
{
    switch(state)
    {
    case STATE::PILOT:
        if(!pilotPulses--)
            state = stateAfterPilot;
        else
            currentPeriod = pilotLength;
        break;
    case STATE::PAUSE:
        if (pauseLength > 0)
            currentPeriod = pauseLength * 1000;
        state = stateAfterPause;
        break;
    case STATE::GETID:
        if (ReadByte())
        {
            blockType = outByte;
            currentBit = 0;
            pass = 0;
            state = STATE::READPARAM;
        } else {
            currentPeriod = 10000;
            EndOfFile = true;
        }
        break;
    case STATE::READPARAM:
        ProcessParams();
        break;
    case STATE::SYNC1:
        currentPeriod = sync1Length;
        state = STATE::SYNC2;
        break;
    case STATE::SYNC2:
        currentPeriod = sync2Length;
        currentBit = 0;
        state = STATE::TDATA;
        break;
    case STATE::TDATA:
        ProcessData();
        break;
    default:
        break;
    }
}

void CDT::ProcessData()
{
    switch(blockType)
    {
    case 0x10:
    case 0x11:
        WriteData();
        break;
    default:
    case 0x13: // Sequence of pulses
        if (!pilotPulses--) {
            state = STATE::GETID;
        } else {
            ReadWord();
            currentPeriod = TicksToMicros(outWord);
        }
        break;
    case 0x14: // Pure data block
        WriteData();
        break;
    case 0x15: // Direct recording
        break;
    }
}

void CDT::WriteData() {
    if ((bytesToRead == 0) && (currentBit == 0) && (pass == 0)) {
        currentPeriod = pauseLength * 1000;
        state = stateAfterPause;
    } else {
        if (currentBit == 0) {
            ReadByte();
            currentByte = outByte;
            bytesToRead--;
            if (bytesToRead)
                currentBit = 8;
            else
                currentBit = usedBitsInLastByte;
            pass = 0;
        }
        currentPeriod = currentByte & 0x80 ? onePulse : zeroPulse;
        pass++;
        if (pass == 2) {
            currentByte <<= 1;
            currentBit--;
            pass = 0;
        }
    }
}

word CDT::TicksToMicros(word ticks) {
    return ticks / 3.5;
}

void CDT::ProcessParams()
{
    switch(blockType)
    {
    case 0x10:
        ReadWord(); pauseLength = outWord;
        ReadWord(); bytesToRead = outWord;
        ReadByte(); pilotPulses = outByte ? PILOTNUMBERH : PILOTNUMBERL;
        bytesRead--;
        pilotLength = PILOTLENGTH;
        sync1Length = SYNCFIRST;
        sync2Length = SYNCSECOND;
        zeroPulse = ZEROPULSE;
        onePulse = ONEPULSE;
        usedBitsInLastByte = 8;
        stateAfterPilot = STATE::SYNC1;
        state = STATE::PILOT;
        break;
    case 0x11:
        ReadWord(); pilotLength = TicksToMicros(outWord);
        ReadWord(); sync1Length = TicksToMicros(outWord);
        ReadWord(); sync2Length = TicksToMicros(outWord);
        ReadWord(); zeroPulse = TicksToMicros(outWord);
        ReadWord(); onePulse = TicksToMicros(outWord);
        ReadWord(); pilotPulses = outWord;
        ReadByte(); usedBitsInLastByte = outByte;
        ReadWord(); pauseLength = outWord;
        ReadLong(); bytesToRead = outDword;
        stateAfterPilot = STATE::SYNC1;
        state = STATE::PILOT;
        break;
    case 0x12:
        //Process ID12 - Pure Tone Block
        ReadWord(); pilotLength = TicksToMicros(outWord);
        ReadWord(); pilotPulses = outWord;
        stateAfterPilot = STATE::GETID;
        state = STATE::PILOT;
        break;
    case 0x13:
        //Process ID13 - Sequence of Pulses
        ReadByte(); pilotPulses = outByte;
        state = STATE::TDATA;
        break;
    case 0x14:
        //process ID14 - Pure Data Block
        ReadWord(); zeroPulse = TicksToMicros(outWord);
        ReadWord(); onePulse = TicksToMicros(outWord);
        ReadByte(); usedBitsInLastByte = outByte;
        ReadWord(); pauseLength = outWord;
        ReadLong(); bytesToRead = outDword;
        state = STATE::TDATA;
        break;
    case 0x15:
        //process ID15 - Direct Recording
        ReadWord();
        ReadWord();
        ReadByte();
        ReadLong(); bytesRead += outDword;
        state = STATE::GETID;
        break;
    case 0x19:
        break;
    case 0x20:
        //process ID20 - Pause Block
        ReadWord(); currentPeriod = outWord; // * 1000;
        state = STATE::GETID;
        break;
    case 0x21: //Process ID21 - Group Start
    case 0x30: //Process ID30 - Text Description
        ReadByte(); bytesRead += outByte;
        state = STATE::GETID;
        break;
    case 0x22:
        //Process ID22 - Group End
        state = STATE::GETID;
        break;
    case 0x24:
        //Process ID24 - Loop Start
        ReadWord(); loopCount = outWord;
        loopStart = bytesRead;
        state = STATE::GETID;
        break;
    case 0x25:
        //Process ID25 - Loop End
        loopCount += -1;
        if (loopCount != 0) {
            bytesRead = loopStart;
        }
        state = STATE::GETID;
        break;
    case 0x2A:
        //Skip//
        bytesRead += 4;
        state = STATE::GETID;
        break;
    case 0x2B:
        //Skip//
        bytesRead += 5;
        state = STATE::GETID;
        break;

    case 0x31:
        //Process ID31 - Message block
        ReadByte();
        ReadByte(); bytesRead += outByte;
        state = STATE::GETID;
        break;
    case 0x32:
        //Process ID32 - Archive Info
        ReadWord(); bytesRead += outWord;
        state = STATE::GETID;
        break;
    case 0x33:
        //Process ID32 - Archive Info
        ReadByte(); bytesRead += (long(outByte) * 3);
        state = STATE::GETID;
        break;
    case 0x35:
        //Process ID35 - Custom Info Block
        bytesRead += 0x10;
        ReadDword();
        bytesRead += outDword;
        state = STATE::GETID;
        break;
    default:
        break;
    }
}



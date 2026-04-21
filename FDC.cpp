#include "FDC.h"
#include <stdio.h>
#include <cstring>

void FDC::Reset()
{
    state = FDCState::FDC_StateCommand;
    commandState = FDCCommandState::FDC_StateCommandCode;
    command = 0;
    HD = false;
    US = PCN = H = R = EOT = 0;
    GPL = DTL = STP = NCN = SC = D = 0;
    SRT = HUT = HLT = 0;
    ND = MT = MF = SK = false;
    headSettlingTime = 0;
    headUploadTimeInterval = 0;
    memset(result, 0, sizeof(result));
    resultCount = 0;
    resultIndex = 0;
    bit7_RQM = 1;
    bit6_DIO = 0;
    bit4_BUSY = 0;
    bit5_NDMA = 0;
    bits03_FDDBUSY[0] = 0;
    bits03_FDDBUSY[1] = 0;
    bits03_FDDBUSY[2] = 0;
    bits03_FDDBUSY[3] = 0;
    dataIndex = 0;
    dataSize = 0;
    sizeCode = 0;
    sectorID = 0;
    weakSectorCycle = 0;
    INT = 4;
    stIC = 0b11000000;
    stSE = stEC = stNR = 0;
    stEN = stDE = stOR = stND = stNW = stMA = 0;
    stCM = stDD = stWC = stSH = stSN = stBC = stMD = 0;
    seekCounter = 0;
    stepPulses = 0;
}

void FDC::Clock()
{
    switch(state)
    {
    case FDCState::FDC_StateExecution:
        ProcessExecution();
        break;
    default:
        break;
    }
}

BYTE FDC::RD_State()
{
    return (bit7_RQM << 7)
           + (bit6_DIO << 6)
           + (bit5_NDMA << 5)
           + (bit4_BUSY << 4)
           + (bits03_FDDBUSY[3] << 3)
           + (bits03_FDDBUSY[2] << 2)
           + (bits03_FDDBUSY[1] << 1)
           + bits03_FDDBUSY[0];
}

BYTE FDC::RD_Data()
{
    if (state == FDCState::FDC_StateTransfer)
    {
        BYTE v = data[dataIndex];
        dataIndex++;
        if (dataIndex == dataSize)
        {
            if (R == EOT)
            {
                R = 1;
                GoToResultState();
            }
            else
            {
                R++;
                GoToExecutionState();
            }
        }
        return v;
    }
    switch(state)
    {
    case FDCState::FDC_StateCommand:
    case FDCState::FDC_StateExecution:
    case FDCState::FDC_StateTransfer:
        return 0x00;
    case FDCState::FDC_StateResult:
        return ProcessResult();
    }
    return 0;
}

void FDC::SetMotor(BYTE value)
{
    MotorState = value;
}

void FDC::WR(BYTE value)
{
    BYTE data = value;
    switch(state)
    {
    case FDCState::FDC_StateCommand:
        ProcessCommand(data);
        break;
    case FDCState::FDC_StateExecution:
        break;
    case FDCState::FDC_StateResult:
        break;
    case FDCState::FDC_StateTransfer:
        break;
    }
}

void FDC::ProcessCommand(BYTE data)
{
    switch(commandState)
    {
    case FDCCommandState::FDC_StateCommandCode:
        MT = data & 0x80;
        MF = data & 0x40;
        SK = data & 0x20;
        command = data & 0x1F;
        switch(command)
        {
        case FDC_CommandReadData:
        case FDC_CommandReadTrack:
        case FDC_CommandReadDeletedData:
        case FDC_CommandReadID:
        case FDC_CommandFormatTrack:
        case FDC_CommandWriteData:
        case FDC_CommandWriteDeletedData:
        case FDC_CommandScanEqual:
        case FDC_CommandScanLowOrEqual:
        case FDC_CommandScanHighOrEqual:
        case FDC_CommandRecalibrate:
        case FDC_CommandSeek:
        case FDC_CommandSenseDriveStatus:
            commandState = FDCCommandState::FDC_StateParam;
            break;
        case FDC_CommandSenseInterruptState:
            GoToExecutionState();
            break;

        case FDC_CommandSpecify:
            commandState = FDCCommandState::FDC_StateSRT_HUT;
            break;
        default:
            GoToExecutionState();
            break;
        }
        break;
    case FDCCommandState::FDC_StateParam:
        HD = data & 0x04;
        US = data & 0x03;
        switch(command)
        {
        case FDC_CommandReadData:
        case FDC_CommandReadTrack:
        case FDC_CommandReadDeletedData:
        case FDC_CommandWriteData:
        case FDC_CommandWriteDeletedData:
        case FDC_CommandScanEqual:
        case FDC_CommandScanLowOrEqual:
        case FDC_CommandScanHighOrEqual:
            commandState = FDCCommandState::FDC_StateC;
            break;
        case FDC_CommandReadID:
        case FDC_CommandRecalibrate:
        case FDC_CommandSenseDriveStatus:
            H = HD ? 1 : 0;
            GoToExecutionState();
            break;
        case FDC_CommandSeek:
            commandState = FDCCommandState::FDC_StateNCN;
            break;
        case FDC_CommandFormatTrack:
            commandState = FDCCommandState::FDC_StateF_N;
            break;
        default:
            break;
        }
        break;
    case FDCCommandState::FDC_StateC:
        PCN = data;
        commandState = FDCCommandState::FDC_StateH;
        break;
    case FDCCommandState::FDC_StateH:
        H = data;
        commandState = FDCCommandState::FDC_StateR;
        break;
    case FDCCommandState::FDC_StateR:
        R = data;
        commandState = FDCCommandState::FDC_StateN;
        break;
    case FDCCommandState::FDC_StateN:
        N = data;
        commandState = FDCCommandState::FDC_StateEOT;
        break;
    case FDCCommandState::FDC_StateEOT:
        EOT = data;
        commandState = FDCCommandState::FDC_StateGPL;
        break;
    case FDCCommandState::FDC_StateGPL:
        GPL = data;
        switch(command)
        {
        case FDC_CommandScanEqual:
        case FDC_CommandScanLowOrEqual:
        case FDC_CommandScanHighOrEqual:
            commandState = FDCCommandState::FDC_StateSTP;
            break;
        default:
            commandState = FDCCommandState::FDC_StateDTL;
            break;
        }
        break;
    case FDCCommandState::FDC_StateDTL:
        DTL = data;
        GoToExecutionState();
        break;
    case FDCCommandState::FDC_StateSTP:
        STP = data;
        GoToExecutionState();
        break;
    case FDCCommandState::FDC_StateNCN:
        NCN = data;
        GoToExecutionState();
        break;
    case FDCCommandState::FDC_StateF_N:
        N = data;
        commandState = FDCCommandState::FDC_StateF_SC;
        break;
    case FDCCommandState::FDC_StateF_SC:
        SC = data;
        commandState = FDCCommandState::FDC_StateF_GPL;
        break;
    case FDCCommandState::FDC_StateF_GPL:
        GPL = data;
        commandState = FDCCommandState::FDC_StateF_D;
        break;
    case FDCCommandState::FDC_StateF_D:
        D = data;
        GoToExecutionState();
        break;
    case FDCCommandState::FDC_StateSRT_HUT:
        SRT = data >> 4;
        HUT = data & 0x0F;
        commandState = FDCCommandState::FDC_StateHLT_ND;
        break;
    case FDCCommandState::FDC_StateHLT_ND:
        HLT = data >> 1;
        ND = data & 0x01;
        stIC = 0b10000000;
        GoToExecutionState();
        break;
    }
}

void FDC::ProcessExecution()
{
    switch(command)
    {
    case FDC_CommandReadData:
    case FDC_CommandReadDeletedData:
        // load head
        // wait head settle time
        stSE = 0b00000000; // Seek End
        sectorInfo = drives[US].GetSectorInfo(PCN, HD ? 1 : 0, R);
        resultCount = 7;
        result[3] = PCN;
        result[4] = H;
        result[5] = R;
        result[6] = N;
        switch(sectorInfo.SI_ID)
        {
        case 0xFE: // Data not found
            stIC = 0b01000000; // Interrupt
            stEC = 0b00000000; // Equipment Check
            stMA = 0b00000001; // Missing Address Mark
            result[0] = GetStatusReg0();
            result[1] = GetStatusReg1();
            result[2] = GetStatusReg2();
            GoToResultState();
            break;
        case 0xFF: // No disc
            stIC = 0b01000000; // Interrupt
            stEC = 0b00010000; // Equipment Check
            result[0] = GetStatusReg0();
            result[1] = GetStatusReg1();
            result[2] = GetStatusReg2();
            GoToResultState();
            break;
        default:
            stIC = 0b00000000; // Interrupt
            stEC = 0b00000000; // Equipment Check
            dataSize = (1 << sectorInfo.SI_size) * 128;
            data = sectorInfo.SectorData[weakSectorCycle++ % sectorInfo.copies];
            dataIndex = 0;
            PCN = sectorInfo.SI_C;
            H = sectorInfo.SI_H;
            N = sectorInfo.SI_size;
            result[0] = GetStatusReg0();
            result[1] = sectorInfo.SI_reg1;
            result[2] = sectorInfo.SI_reg2;
            GoToTransferState();
            break;
        }
        break;
    case FDC_CommandReadTrack:
    case FDC_CommandWriteData:
    case FDC_CommandWriteDeletedData:
    case FDC_CommandScanEqual:
    case FDC_CommandScanLowOrEqual:
    case FDC_CommandScanHighOrEqual:
    case FDC_CommandFormatTrack:
        if (stNR)
            stIC = 0b01000000;
        else
            stIC = 0b00000000;
        if (command == FDC_CommandReadTrack)
            stEN = 0b10000000;
        resultCount = 7;
        result[0] = GetStatusReg0();
        result[1] = GetStatusReg1();
        result[2] = GetStatusReg2();
        result[3] = PCN;
        result[4] = H;
        result[5] = R;
        result[6] = N;
        GoToResultState();
        break;
    case FDC_CommandRecalibrate:
    case FDC_CommandSeek:
    {
        BYTE target = (command == FDC_CommandRecalibrate) ? 0 : NCN;
        if (!stNR && PCN != target)
        {
            if (seekCounter < 48000)
            {
                seekCounter++;
                break;
            }
            seekCounter = 0;
            if (PCN < target) PCN++;
            else PCN--;
            stepPulses++;
            if (PCN != target)
                break;
        }
        INT++;
        if (stNR)
        {
            stSE = 0b00100000;
            stIC = 0b01000000;
            stEC = 0b00010000;
        }
        else
        {
            stIC = 0b00000000;
            stSE = 0b00100000;
            stEC = 0b00000000;
        }
        GoToCommandState();
        break;
    }
    case FDC_CommandSenseInterruptState:
        resultCount = 2;
        if (!INT)
            stIC = 0b10000000;
        result[0] = GetStatusReg0();
        result[1] = PCN;
        if (INT > 0)
            INT--;
        GoToResultState();
        break;
    case FDC_CommandSpecify:
        GoToCommandState();
        break;
    case FDC_CommandReadID: {
        BYTE firstID = drives[US].GetSectorID(PCN, H);
        SectorInfo si = drives[US].GetSectorInfo(PCN, H, firstID);
        if (firstID == 0xFF || firstID == 0x00 || stNR)
            stIC = 0b01000000;
        else
            stIC = 0b00000000;
        resultCount = 7;
        result[0] = GetStatusReg0();
        result[1] = GetStatusReg1();
        result[2] = GetStatusReg2();
        result[3] = si.SI_C;
        result[4] = si.SI_H;
        result[5] = si.SI_ID;
        result[6] = si.SI_size;
        GoToResultState();
        break;
    }
    case FDC_CommandSenseDriveStatus:
        resultCount = 1;
        result[0] = GetStatusReg3();
        GoToResultState();
        break;
    default:
        stIC = 0b10000000;
        resultCount = 1;
        result[0] = command | 0x80;
        GoToResultState();
        break;
    }
}

BYTE FDC::ProcessResult()
{
    if (resultCount)
    {
        BYTE value = result[resultIndex];
        resultIndex++;
        if (resultIndex == resultCount)
        {
            if (command == FDC_CommandSenseInterruptState)
                stSE = 0;
            GoToCommandState();
        }
        return value;
    }
    else
        GoToCommandState();
    return 0;
}

FloppyDrive *FDC::GetDrive(int number)
{
    return &drives[number];
}

void FDC::GoToCommandState()
{
    bit7_RQM = 1;
    bit6_DIO = 0;
    bit4_BUSY = 0;
    bit5_NDMA = 0;
    bits03_FDDBUSY[US] = 0;
    resultIndex = 0;
    state = FDCState::FDC_StateCommand;
    commandState = FDCCommandState::FDC_StateCommandCode;
}

static const char* FDC_CommandName(BYTE cmd)
{
    switch(cmd & 0x1F) {
    case 0x06: return "ReadData";
    case 0x0C: return "ReadDeletedData";
    case 0x02: return "ReadTrack";
    case 0x0A: return "ReadID";
    case 0x0D: return "FormatTrack";
    case 0x05: return "WriteData";
    case 0x09: return "WriteDeletedData";
    case 0x11: return "ScanEqual";
    case 0x19: return "ScanLowOrEqual";
    case 0x1D: return "ScanHighOrEqual";
    case 0x07: return "Recalibrate";
    case 0x0F: return "Seek";
    case 0x08: return "SenseInterrupt";
    case 0x04: return "SenseDriveStatus";
    case 0x03: return "Specify";
    default:   return "Unknown";
    }
}

void FDC::GoToExecutionState()
{
    stNR = drives[US].DiskInserted ? 0 :  0b00001000; // Not Ready
    stEN = 0; // End of Cylinder
    stDE = 0; // Data Error
    stOR = 0; // Overrun
    stND = 0; // No Data
    stNW = 0; // Now Writeable
    stMA = 0; // Missing Address mark
    stCM = 0; // Control Mark
    stDD = 0; // Data error in Data field
    stWC = 0; // Wrong Cylinder
    stSH = 0; // Scan equal Hit
    stSN = 0; // Scan Not satisfied
    stBC = 0; // Bad Cylinder
    stMD = 0; // Missing address mark in Data field
    bit7_RQM = 0;
    bit4_BUSY = 1;
    bit5_NDMA = 1;
    bits03_FDDBUSY[US] = 1;
    if (command == FDC_CommandSeek || command == FDC_CommandRecalibrate)
        seekCounter = 0;
    state = FDCState::FDC_StateExecution;
    printf("[FDC] CMD=%s(0x%02X) US=%d HD=%d PCN=%d H=%d R=%d N=%d EOT=%d NCN=%d\n",
           FDC_CommandName(command), command, US, (int)HD, PCN, H, R, N, EOT, NCN);
}

void FDC::GoToTransferState()
{
    bit7_RQM = 1;
    bit6_DIO = 1;
    state = FDCState::FDC_StateTransfer;
}

void FDC::GoToResultState()
{
    bit7_RQM = 1;
    bit6_DIO = 1;
    bit5_NDMA = 0;
    bits03_FDDBUSY[US] = 0;
    state = FDCState::FDC_StateResult;
    printf("[FDC] RESULT(%s):", FDC_CommandName(command));
    for (int i = 0; i < resultCount; i++) printf(" %02X", result[i]);
    printf("\n");
}

BYTE FDC::GetStatusReg0()
{
    return stIC + stSE + stEC + stNR + H * 4 + US;
}

BYTE FDC::GetStatusReg1()
{
    return stEN + stDE + stOR + stND + stNW + stMA;
}

BYTE FDC::GetStatusReg2()
{
    return stCM + stDD + stWC + stSH + stSN + stBC + stMD;
}

BYTE FDC::GetStatusReg3()
{
    BYTE ts = (drives[US].GetSides() > 1) ? 0b00001000 : 0;
    return (stNR ? 0 : 0b00100000) + (PCN ? 0 : 0b00010000) + ts + ((H & 1) << 2) + US;
}

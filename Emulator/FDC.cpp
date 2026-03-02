#include "Headers/FDC.h"
#include "Headers/Z80.h"

FDCState FDC::state;
FDCCommandState FDC::commandState;
BYTE FDC::command;
BYTE FDC::PCN;
BYTE FDC::H;
BYTE FDC::R;
BYTE FDC::N;
BYTE FDC::EOT;
BYTE FDC::GPL;
BYTE FDC::DTL;
BYTE FDC::STP;
BYTE FDC::NCN;
BYTE FDC::SC;
BYTE FDC::D;
BYTE FDC::SRT;
BYTE FDC::HUT;
BYTE FDC::HLT;
bool FDC::ND;
bool FDC::MT;
bool FDC::MF;
bool FDC::SK;
FloppyDrive FDC::drives[4];
BYTE FDC::headSettlingTime;
BYTE FDC::headUploadTimeInterval;
bool FDC::HD;
BYTE FDC::US;
BYTE FDC::result[7];
BYTE FDC::resultCount;
BYTE FDC::resultIndex;
bool FDC::bit7_RQM;
bool FDC::bit6_DIO;
bool FDC::bit5_NDMA;
bool FDC::bit4_BUSY;
bool FDC::bits03_FDDBUSY[4];
SectorInfo FDC::sectorInfo;
int FDC::dataIndex;
int FDC::dataSize;
BYTE FDC::sizeCode;
BYTE FDC::sectorID;
BYTE *FDC::data;
BYTE FDC::weakSectorCycle;
BYTE FDC::INT;
BYTE FDC::stIC, FDC::stSE, FDC::stEC, FDC::stNR;
BYTE FDC::stEN, FDC::stDE, FDC::stOR, FDC::stND, FDC::stNW, FDC::stMA;
BYTE FDC::stCM, FDC::stDD, FDC::stWC, FDC::stSH, FDC::stSN, FDC::stBC, FDC::stMD;


void FDC::Reset()
{
    // Set default status reg 0
    stIC = 0b11000000;
    stSE = stEC = stNR = 0;
    H = 0;
    US = 0;
    // Set default status reg 1
    stEN = stDE = stOR = stND = stNW = stMA = 0;
    // Set default status reg 2
    stCM = stDD = stWC = stSH = stSN = stBC = stMD = 0;

    INT = 4;
    headSettlingTime = 0;
    headUploadTimeInterval = 0;
    weakSectorCycle = 0;
    GoToCommandState();
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

void FDC::RD()
{
    if ((Z80::AR & 0x0100) != 0)
    {
        if ((Z80::AR & 0x0001) == 0)
        {
            Z80::DR = (bit7_RQM << 7)
                      + (bit6_DIO << 6)
                      + (bit5_NDMA << 5)
                      + (bit4_BUSY << 4)
                      + (bits03_FDDBUSY[3] << 3)
                      + (bits03_FDDBUSY[2] << 2)
                      + (bits03_FDDBUSY[1] << 1)
                      + bits03_FDDBUSY[0];
        }
        else
        {
            if (state == FDCState::FDC_StateTransfer)
            {
                Z80::DR = data[dataIndex];
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
                return;
            }
            switch(state)
            {
            case FDCState::FDC_StateCommand:
            case FDCState::FDC_StateExecution:
            case FDCState::FDC_StateTransfer:
                Z80::DR = 0x00;
                break;
            case FDCState::FDC_StateResult:
                ProcessResult();
                break;
            }
        }
    }
}

void FDC::WR()
{
    BYTE data = Z80::DR;
    if ((Z80::AR & 0x0100) == 0)
    {
        // Set motor
    }
    else if ((Z80::AR & 0x0001) != 0)
    {
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
            commandState = FDCCommandState::FDC_StateParam;
            break;
        case FDC_CommandSenseDriveStatus:
            resultCount = 1;
            result[0] = GetStatusReg3();
            GoToResultState();
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
        sectorInfo = drives[US].GetSectorInfo(PCN, R);
        if (sectorInfo.SI_C != 0xFF)
        {
            dataSize = sectorInfo.SI_size * 256;
            data = sectorInfo.SectorData;
            if (sectorInfo.copies > 1)
                data += (weakSectorCycle++ % sectorInfo.copies) * dataSize;
            dataIndex = 0;

            PCN = sectorInfo.SI_C;
            H = sectorInfo.SI_H;
            N = sectorInfo.SI_size;
            stIC = 0b00000000; // Interrupt
            stEC = 0b00000000; // Equipment Check
            resultCount = 7;
            result[0] = GetStatusReg0();
            result[1] = sectorInfo.SI_reg1;
            result[2] = sectorInfo.SI_reg2;
        }
        else
        {
            stIC = 0b01000000;
            stEC = 0b00010000; // Equipment Check
            result[0] = GetStatusReg0();
            result[1] = GetStatusReg1();
            result[2] = GetStatusReg2();
        }
        result[3] = PCN;
        result[4] = H;
        result[5] = R;
        result[6] = N;
        GoToTransferState();
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
        INT++;
        PCN = 0;
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
    case FDC_CommandSeek:
        INT++;
        PCN = NCN;
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
    case FDC_CommandReadID:
        R = drives[US].GetSectorID(PCN);
        if (R == 0 || stNR)
            stIC = 0b01000000;
        else
            stIC = 0b00000000;
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

void FDC::ProcessResult()
{
    if (resultCount)
    {
        Z80::DR = result[resultIndex];
        resultIndex++;
        if (resultIndex == resultCount)
        {
            if (command == FDC_CommandSenseInterruptState)
                stSE = 0;
            GoToCommandState();
        }
    }
    else
        GoToCommandState();
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
    state = FDCState::FDC_StateExecution;
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
    return 0b00000000 + (stNR ? 0 : 0b00100000) + (PCN ? 0 : 0b00010000) + H * 4 + US;
}

#include "Headers/FDC.h"
#include "Headers/Z80.h"

FDCState FDC::state;
FDCCommandState FDC::commandState;
BYTE FDC::command;
BYTE FDC::C;
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
BYTE FDC::statusReg0;
BYTE FDC::statusReg1;
BYTE FDC::statusReg2;
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
word FDC::executionDelay;
SectorInfo FDC::sectorInfo;
int FDC::dataIndex;
int FDC::dataSize;
BYTE FDC::sizeCode;
BYTE FDC::sectorID;
BYTE *FDC::data;
BYTE FDC::weakSectorCycle;


void FDC::Reset()
{
    executionDelay = 0;
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
                        GoToExecutionState(0);
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
        case FDC_CommandSenseDriveStatus:
        case FDC_CommandSeek:
            commandState = FDCCommandState::FDC_StateParam;
            break;
        case FDC_CommandSenseInterruptState:
            GoToExecutionState(20);
            break;
        case FDC_CommandSpecify:
            commandState = FDCCommandState::FDC_StateSRT_HUT;
            break;
        default:
            GoToExecutionState(20);
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
            GoToExecutionState(100);
            break;
        case FDC_CommandSeek:
            commandState = FDCCommandState::FDC_StateNCN;
            break;
        case FDC_CommandFormatTrack:
            commandState = FDCCommandState::FDC_StateF_N;
            break;
        default:
            // Should never happen
            break;
        }
        break;
    case FDCCommandState::FDC_StateC:
        C = data;
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
        GoToExecutionState(100);
        break;
    case FDCCommandState::FDC_StateSTP:
        STP = data;
        GoToExecutionState(100);
        break;
    case FDCCommandState::FDC_StateNCN:
        NCN = data;
        GoToExecutionState(100);
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
        GoToExecutionState(100);
        break;
    case FDCCommandState::FDC_StateSRT_HUT:
        SRT = data >> 4;
        HUT = data & 0x0F;
        commandState = FDCCommandState::FDC_StateHLT_ND;
        break;
    case FDCCommandState::FDC_StateHLT_ND:
        HLT = data >> 1;
        ND = data & 0x01;
        GoToExecutionState(100);
        break;
    }
}

void FDC::ProcessExecution()
{
    /*
    if (executionDelay > 0)
    {
        executionDelay--;
        return;
    }
*/
    switch(command)
    {
    case FDC_CommandReadData:
    case FDC_CommandReadDeletedData:
        // load head
        // wait head settle time
        sectorInfo = drives[US].GetSectorInfo(C, R);
        if (sectorInfo.SI_C != 0xFF)
        {
            dataSize = sectorInfo.SI_size * 256;
            data = sectorInfo.SectorData;
            if (sectorInfo.copies > 1)
                data += (weakSectorCycle++ % sectorInfo.copies) * dataSize;
            dataIndex = 0;

            statusReg0 = 0x00;
            statusReg1 = sectorInfo.SI_reg1;
            statusReg2 = sectorInfo.SI_reg2;
            C = sectorInfo.SI_C;
            H = sectorInfo.SI_H;
            N = sectorInfo.SI_size;

            resultCount = 7;
            result[0] = statusReg0;
            result[1] = statusReg1;
            result[2] = statusReg2;
            result[3] = C;
            result[4] = H;
            result[5] = R;
            result[6] = N;
            GoToTransferState();
        }
        break;
    case FDC_CommandReadTrack:
    case FDC_CommandWriteData:
    case FDC_CommandWriteDeletedData:
    case FDC_CommandScanEqual:
    case FDC_CommandScanLowOrEqual:
    case FDC_CommandScanHighOrEqual:
    case FDC_CommandFormatTrack:
        resultCount = 7;
        result[0] = statusReg0;
        result[1] = statusReg1;
        result[2] = statusReg2;
        result[3] = C;
        result[4] = H;
        result[5] = R;
        result[6] = N;
        GoToResultState();
        break;
    case FDC_CommandRecalibrate:
        C = 0;
        GoToCommandState();
        break;
    case FDC_CommandSeek:
        C = NCN;
        GoToCommandState();
        break;
    case FDC_CommandSenseInterruptState:
        resultCount = 2;
        result[0] = 0b00100000;
        result[1] = C;
        GoToResultState();
        break;
    case FDC_CommandSpecify:
        GoToCommandState();
        break;
    case FDC_CommandReadID:
        R = drives[US].GetSectorID(C);

        statusReg0 = 0x00;
        statusReg1 = 0x00;
        statusReg2 = 0x00;

        resultCount = 7;
        result[0] = statusReg0;
        result[1] = statusReg1;
        result[2] = statusReg2;
        result[3] = C;
        result[4] = H;
        result[5] = R;
        result[6] = N;
        GoToResultState();
        break;
    default:
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
            GoToCommandState();
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

void FDC::GoToExecutionState(int length)
{
    executionDelay = length;
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

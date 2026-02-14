#include "Headers/FDC.h"
#include "Headers/CPC.h"

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
BYTE FDC::mainStatusReg;
BYTE FDC::head;

void FDC::Reset()
{
    mainStatusReg = 0b10000000;
    GoToCommandState();
    headSettlingTime = 0;
    head = 0;
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

void FDC::Clock_IO_RD()
{
    if ((CPC::AddressBUS & 0x0100) != 0)
    {
        if ((CPC::AddressBUS & 0x0001) == 0)
        {
            switch(state)
            {
            case FDCState::FDC_StateCommand:
                CPC::DataBUS = mainStatusReg;
                break;
            case FDCState::FDC_StateExecution:
                break;
            case FDCState::FDC_StateResult:
                ProcessResult();
                break;
            }
        }
        else
        {
            // Read data
        }
    }
}

void FDC::Clock_IO_WR()
{
    BYTE data = CPC::DataBUS;
    if ((CPC::AddressBUS & 0x0100) == 0)
    {
        // Set motor
    }
    else if ((CPC::AddressBUS & 0x0001) != 0)
    {
        ProcessInput(data);
    }
}

void FDC::ProcessInput(BYTE data)
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
            state = FDCState::FDC_StateExecution;
            break;
        case FDC_CommandSpecify:
            commandState = FDCCommandState::FDC_StateSRT_HUT;
            break;
        default:
            // Invalid codes: Do nothing
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
            state = FDCState::FDC_StateExecution;
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
        state = FDCState::FDC_StateExecution;
        break;
    case FDCCommandState::FDC_StateSTP:
        STP = data;
        state = FDCState::FDC_StateExecution;
        break;
    case FDCCommandState::FDC_StateNCN:
        NCN = data;
        state = FDCState::FDC_StateExecution;
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
        state = FDCState::FDC_StateExecution;
        break;
    case FDCCommandState::FDC_StateSRT_HUT:
        SRT = data >> 4;
        HUT = data & 0x0F;
        commandState = FDCCommandState::FDC_StateHLT_ND;
        break;
    case FDCCommandState::FDC_StateHLT_ND:
        HLT = data >> 1;
        ND = data & 0x01;
        state = FDCState::FDC_StateExecution;
        break;
    }
}

void FDC::ProcessExecution()
{
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
    case FDC_CommandFormatTrack:
        resultCount = 7;
        result[0] = statusReg0;
        result[1] = statusReg1;
        result[2] = statusReg2;
        result[3] = C;
        result[4] = H;
        result[5] = R;
        result[6] = N;
        state = FDCState::FDC_StateResult;
        break;
    case FDC_CommandRecalibrate:
        head = 0;
        statusReg0 = 0b00100000;
        statusReg1 = head;
        GoToCommandState();
        break;
    case FDC_CommandSenseInterruptState:
        resultCount = 2;
        result[0] = statusReg0;
        result[1] = statusReg1;
        state = FDCState::FDC_StateResult;
        break;
    default:
        break;
    }
}

void FDC::ProcessResult()
{
    if (resultCount)
    {
        CPC::DataBUS = result[resultIndex];
        resultIndex++;
        if (resultIndex == resultCount)
            GoToCommandState();
    }
    else
        GoToCommandState();
}

FloppyDrive FDC::GetDrive(int number)
{
    return drives[number];
}

void FDC::GoToCommandState()
{
    resultIndex = 0;
    state = FDCState::FDC_StateCommand;
    commandState = FDCCommandState::FDC_StateCommandCode;
}

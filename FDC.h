#ifndef FDC_H
#define FDC_H

#include "defs.h"
#include "FloppyDrive.h"
#include <atomic>

enum FDCState
{
    FDC_StateCommand,
    FDC_StateExecution,
    FDC_StateTransfer,
    FDC_StateResult,
};

enum FDCCommandState
{
    FDC_StateCommandCode,
    FDC_StateParam,
    FDC_StateC,
    FDC_StateR,
    FDC_StateH,
    FDC_StateN,
    FDC_StateEOT,
    FDC_StateGPL,
    FDC_StateDTL,
    FDC_StateSTP,
    FDC_StateNCN,
    FDC_StateF_N,
    FDC_StateF_SC,
    FDC_StateF_GPL,
    FDC_StateF_D,
    FDC_StateSRT_HUT,
    FDC_StateHLT_ND
};

#define FDC_CommandInvalid 0
#define FDC_CommandReadData 6
#define FDC_CommandReadID 10
#define FDC_CommandReadDeletedData 12
#define FDC_CommandReadTrack 2
#define FDC_CommandScanEqual 17
#define FDC_CommandScanHighOrEqual 29
#define FDC_CommandScanLowOrEqual 25
#define FDC_CommandSpecify 3
#define FDC_CommandWriteData 5
#define FDC_CommandFormatTrack 13
#define FDC_CommandWriteDeletedData 9
#define FDC_CommandSeek 15
#define FDC_CommandRecalibrate 7
#define FDC_CommandSenseInterruptState 8
#define FDC_CommandSenseDriveStatus 4

class FDC
{
public:
    void Reset();
    void Clock();
    BYTE RD_State();
    BYTE RD_Data();
    void WR(BYTE value);
    void SetMotor(BYTE value);
    bool GetMotor() { return MotorState; }
    FDCState GetState() { return state; }
    FloppyDrive *GetDrive(int number);
private:
    void ProcessCommand(BYTE data);
    void ProcessExecution();
    BYTE ProcessResult();
    void GoToCommandState();
    void GoToExecutionState();
    void GoToTransferState();
    void GoToResultState();
    BYTE GetStatusReg0();
    BYTE GetStatusReg1();
    BYTE GetStatusReg2();
    BYTE GetStatusReg3();
    FDCState state;
    FDCCommandState commandState;
    BYTE command;
    bool HD;
    BYTE US;
    BYTE PCN, H, R, N, EOT, GPL, DTL, STP, NCN, SC, D;
    BYTE SRT, HUT, HLT;
    bool ND;
    bool MT;
    bool MF;
    bool SK;
    FloppyDrive drives[4];
    BYTE headSettlingTime;
    BYTE headUploadTimeInterval;
    BYTE result[7];
    BYTE resultCount;
    BYTE resultIndex;
    bool bit7_RQM;
    bool bit6_DIO;
    bool bit5_NDMA;
    bool bit4_BUSY;
    bool bits03_FDDBUSY[4];
    SectorInfo sectorInfo;
    int dataIndex;
    int dataSize;
    BYTE sizeCode;
    BYTE sectorID;
    BYTE *data;
    BYTE weakSectorCycle;
    int seekCounter;
    BYTE formatBuffer[256];
    int formatByteCount;
    bool scanSectorMatch;
    bool readTerminateAfterSector;
    BYTE physTrack;
    BYTE physSide;
    BYTE trackReadPosition;
    BYTE trackReadSectorsDone;
    bool trackReadNoDataFlag;
public:
    std::atomic<int> stepPulses{0};
private:
    BYTE INT;
    BYTE stIC, stSE, stEC, stNR; // For Reg0
    BYTE stEN, stDE, stOR, stND, stNW, stMA; // For Reg1
    BYTE stCM, stDD, stWC, stSH, stSN, stBC, stMD; // For Reg2
    bool MotorState;
};

#endif // FDC_H

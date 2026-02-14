#ifndef FDC_H
#define FDC_H

#include "defs.h"
#include "FloppyDrive.h"

enum FDCState
{
    FDC_StateCommand,
    FDC_StateExecution,
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
    static void Reset();
    static void Clock();
    static void Clock_IO_RD();
    static void Clock_IO_WR();
    static FloppyDrive *GetDrive(int number);
private:
    static void ProcessCommand(BYTE data);
    static void ProcessExecution();
    static void ProcessResult();
    static void GoToCommandState();
    static void GoToExecutionState();
    static void GoToResultState();
    static FDCState state;
    static FDCCommandState commandState;
    static BYTE command;
    static bool HD;
    static BYTE US;
    static BYTE C, H, R, N, EOT, GPL, DTL, STP, NCN, SC, D;
    static BYTE SRT, HUT, HLT;
    static bool ND;
    static bool MT;
    static bool MF;
    static bool SK;
    static FloppyDrive drives[4];
    static BYTE headSettlingTime;
    static BYTE headUploadTimeInterval;
    static BYTE statusReg0;
    static BYTE statusReg1;
    static BYTE statusReg2;
    static BYTE statusReg3;
    static BYTE result[7];
    static BYTE resultCount;
    static BYTE resultIndex;
    static BYTE head;
    static bool bit7_RQM;
    static bool bit6_DIO;
    static bool bit5_NDMA;
    static bool bit4_BUSY;
    static bool bits03_FDDBUSY[4];
    static word executionDelay;
    static BYTE *sectorData;
};

#endif // FDC_H

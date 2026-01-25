#ifndef EMULATORWORKERTHREAD_H
#define EMULATORWORKERTHREAD_H

#include <QThread>
#include <string>
#include <mutex>

using namespace std;

class EmulatorWorkerThread : public QThread
{
    Q_OBJECT
public:
    explicit EmulatorWorkerThread(QObject *parent);
    static string debugStringZ80;
    static string debugStringStack;
    static string debugStringDisassembler;
    static string debugStringCRTC;
    static string debugStringGateArray;
    static string debugStringMem;
    static void Run();
    static void Pause();
    static void StepIn();
    static void StepOut();
    static void StepOver();
    static volatile bool running;
    static volatile ushort stopPoint;
    static volatile bool stepByStep;
    static volatile int iteration;
    static volatile int measures;
    static volatile int total;
    static mutex debugLock;
signals:
    void OnPause();
protected:
    void run() override;
private:
    static string GetZ80RegsDebugLine();
    static string GetZ80StackDebugLine();
    static string GetCRTCDebugLine();
    static string GetDisassembly();
    static string GetGateArrayDebugLine();
    static string GetMemDebugLine();
    static void StartDebugging();
    static volatile bool canDebug;
    static unsigned char nextInstructionLength;
};

#endif // EMULATORWORKERTHREAD_H

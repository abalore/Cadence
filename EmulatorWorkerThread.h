#ifndef EMULATORWORKERTHREAD_H
#define EMULATORWORKERTHREAD_H

#include <QThread>
#include <string>

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
    static volatile bool canDebug;
};

#endif // EMULATORWORKERTHREAD_H

#ifndef EMULATORWORKERTHREAD_H
#define EMULATORWORKERTHREAD_H

#include <QThread>
#include <string>
#include <mutex>

using namespace std;

enum RunMode
{
    Run,
    StepByStep,
    StopPoint
};

class EmulatorWorkerThread : public QThread
{
    Q_OBJECT
public:
    explicit EmulatorWorkerThread(QObject *parent);
    static string debugStringZ80;
    static string debugStringStack;
    static string debugStringCRTC;
    static string debugStringGateArray;
    static void Run();
    static void RunStep();
    static void RunTo(ushort address);
    static void Reset();
    static volatile bool running;
    static volatile ushort stopPoint;
    static volatile bool end;
    static volatile long elapsed;
    static RunMode runMode;
    static mutex debugLock;
signals:
    void OnPause();
    void OnFinishedFrame();
protected:
    void run() override;
private:
    static string GetZ80RegsDebugLine();
    static string GetZ80StackDebugLine();
    static string GetCRTCDebugLine();
    static string GetGateArrayDebugLine();
    void Stop();
    static volatile bool canDebug;
    static unsigned char nextInstructionLength;
};

#endif // EMULATORWORKERTHREAD_H

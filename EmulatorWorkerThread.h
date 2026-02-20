#ifndef EMULATORWORKERTHREAD_H
#define EMULATORWORKERTHREAD_H

#include <QThread>
#include <string>

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
    ~EmulatorWorkerThread();
    static volatile bool running;
    static volatile ushort stopPoint;
    static volatile bool end;
    static volatile RunMode runMode;
public slots:
    static void Run();
    static void RunStep();
    static void RunTo(ushort address);
    static void Reset();
    static void Pause();
protected:
    void run() override;
signals:
    void OnPause();
    void OnFinishedFrame();
private:
    void Stop();
};

#endif // EMULATORWORKERTHREAD_H

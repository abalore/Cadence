#ifndef EMULATORWORKERTHREAD_H
#define EMULATORWORKERTHREAD_H

#include <QThread>
#include <QMutex>
#include <atomic>
#include <string>

using namespace std;

enum RunMode
{
    Run,
    StepByStep,
    StopPoint
};

class EmulatorThread : public QThread
{
    Q_OBJECT
public:
    explicit EmulatorThread(QObject *parent);
    ~EmulatorThread();
    static std::atomic<bool> running;
    static std::atomic<ushort> stopPoint;
    static std::atomic<bool> end;
    static std::atomic<RunMode> runMode;
    static QMutex frameMutex;
public slots:
    static void Run();
    static void RunStep();
    static void RunTo(ushort address);
    static void Pause();
protected:
    void run() override;
signals:
    void OnPause();
    void OnResume();
    void OnFinishedFrame();
private:
    void Stop();
    bool paused;
};

#endif // EMULATORWORKERTHREAD_H

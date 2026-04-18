#ifndef EMULATORWORKERTHREAD_H
#define EMULATORWORKERTHREAD_H

#include <QThread>
#include <QMutex>
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
    static volatile bool running;
    static volatile ushort stopPoint;
    static volatile bool end;
    static volatile RunMode runMode;
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

#ifndef SOUNDTHREAD_H
#define SOUNDTHREAD_H

#include <QThread>
#include <QAudioSink>

class SoundThread : public QThread
{
    Q_OBJECT
public:
    explicit SoundThread(QObject *parent);
    ~SoundThread();
    volatile bool end;
signals:
protected:
    void run() override;
private:
    QAudioSink *output;
};

#endif // SOUNDTHREAD_H

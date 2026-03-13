#ifndef SOUNDTHREAD_H
#define SOUNDTHREAD_H

#include <QThread>
#include <QWaitCondition>
#include <alsa/asoundlib.h>

class SoundThread : public QThread
{
    Q_OBJECT;
public:
    explicit SoundThread(QObject *parent);
    ~SoundThread();
    volatile bool end;
    static QWaitCondition waitCondition;
    volatile static snd_pcm_sframes_t frames;
protected:
    void run() override;
private:
    snd_pcm_t *pcm_handle;
    static snd_pcm_sframes_t frames_internal;
};

#endif // SOUNDTHREAD_H

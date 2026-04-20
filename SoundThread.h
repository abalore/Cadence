#ifndef SOUNDTHREAD_H
#define SOUNDTHREAD_H

#include <QThread>
#include <QWaitCondition>
#include <alsa/asoundlib.h>
#include "defs.h"

class SoundThread : public QThread
{
    Q_OBJECT;
public:
    explicit SoundThread(QObject *parent);
    ~SoundThread();
    volatile bool end;
    static volatile bool enabled;
    static volatile bool sfxEnabled;
    static QWaitCondition waitCondition;
    volatile static snd_pcm_sframes_t frames;
protected:
    void run() override;
private:
    snd_pcm_t *pcm_handle;
    static snd_pcm_sframes_t frames_internal;
    BYTE *stepBuffer;
    int stepBufferLen;
    int stepPos;
    BYTE *spinBuffer;
    int spinBufferLen;
    int spinPos;
};

#endif // SOUNDTHREAD_H

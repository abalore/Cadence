#ifndef SOUNDTHREAD_H
#define SOUNDTHREAD_H

#include <QThread>
#include <QWaitCondition>
#include <alsa/asoundlib.h>
#include <atomic>
#include "defs.h"

class SoundThread : public QThread
{
    Q_OBJECT;
public:
    explicit SoundThread(QObject *parent);
    ~SoundThread();
    std::atomic<bool> end{false};
    static std::atomic<bool> enabled;
    static std::atomic<bool> sfxEnabled;
    static QWaitCondition waitCondition;
    static std::atomic<snd_pcm_sframes_t> frames;
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

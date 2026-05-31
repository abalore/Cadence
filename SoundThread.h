#ifndef SOUNDTHREAD_H
#define SOUNDTHREAD_H

#include <QThread>
#include <QWaitCondition>
#include <portaudio.h>
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
    static std::atomic<long> frames;
    static std::atomic<SoundThread *> instance;
    static int bufferFrames;   // PortAudio framesPerBuffer / PipeWire quantum; read at construction
    void pushFrame();
protected:
    void run() override;
private:
    PaStream *stream;
    bool paInitialized;

    static constexpr int RING_SIZE = 8192;
    static BYTE ringBuffer[RING_SIZE];
    static std::atomic<int> ringHead;
    static std::atomic<int> ringTail;

    static int paCallback(const void *input, void *output,
                          unsigned long frameCount,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData);

    BYTE *stepBuffer;
    int stepBufferLen;
    int stepPos;
    BYTE *spinBuffer;
    int spinBufferLen;
    int spinPos;
};

#endif // SOUNDTHREAD_H

#include "SoundThread.h"
#include "PSG.h"
#include <QMutex>
#include <cstring>

QWaitCondition SoundThread::waitCondition;
volatile snd_pcm_sframes_t SoundThread::frames;
snd_pcm_sframes_t SoundThread::frames_internal;
volatile bool SoundThread::enabled = true;

SoundThread::SoundThread(QObject *parent) : QThread(parent)
{
    unsigned int rate = 62500;
    snd_pcm_uframes_t period_size = 64;
    snd_pcm_uframes_t buffer_size = 64 * 64;
    snd_pcm_hw_params_t *params;
    snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(pcm_handle, params);
    snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_U8);
    snd_pcm_hw_params_set_channels(pcm_handle, params, 1);
    snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, 0);
    snd_pcm_hw_params_set_buffer_size_near(pcm_handle, params, &buffer_size);
    snd_pcm_hw_params_set_period_size_near(pcm_handle, params, &period_size, 0);
    snd_pcm_hw_params(pcm_handle, params);
    end = false;
}

SoundThread::~SoundThread()
{
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
}

void SoundThread::run()
{
    QMutex mutex;
    mutex.lock();
    while (!end)
    {
        waitCondition.wait(&mutex);
        if (!enabled)
            memset(PSG::buffer, 0x80, PSG::bufferIndex);
        BYTE *p = PSG::buffer;
        int remaining = PSG::bufferIndex;
        while (remaining > 0) {
            snd_pcm_sframes_t wr = snd_pcm_writei(pcm_handle, p, remaining);
            if (wr == -EPIPE) {
                snd_pcm_prepare(pcm_handle);
                continue;
            }
            if (wr == -EAGAIN || wr == -EINTR) continue;
            if (wr < 0) break;
            p += wr;
            remaining -= wr;
        }
        PSG::bufferIndex = 0;
        snd_pcm_delay(pcm_handle, &frames_internal);
        frames = frames_internal;
    }
    mutex.unlock();
}

#include "SoundThread.h"
#include "PSG.h"
#include "FDC.h"
#include <QMutex>
#include <QFile>
#include <QByteArray>
#include <cstring>
#include <cstdio>
#include <algorithm>

QWaitCondition SoundThread::waitCondition;
volatile snd_pcm_sframes_t SoundThread::frames;
snd_pcm_sframes_t SoundThread::frames_internal;
volatile bool SoundThread::enabled = true;
volatile bool SoundThread::sfxEnabled = true;

static void loadWavU8(const char *path, BYTE *&buffer, int &length)
{
    buffer = nullptr;
    length = 0;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return;
    QByteArray all = f.readAll();
    int idx = all.indexOf("data", 12);
    if (idx < 0 || idx + 8 > all.size()) return;
    const unsigned char *p = (const unsigned char *)all.constData() + idx + 4;
    int dataSize = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
    if (dataSize <= 0 || idx + 8 + dataSize > all.size()) return;
    buffer = new BYTE[dataSize];
    memcpy(buffer, all.constData() + idx + 8, dataSize);
    length = dataSize;
}

SoundThread::SoundThread(QObject *parent) : QThread(parent)
{
    loadWavU8(":/images/step.wav", stepBuffer, stepBufferLen);
    stepPos = stepBufferLen;
    loadWavU8(":/images/spin.wav", spinBuffer, spinBufferLen);
    spinPos = 0;

    unsigned int rate = 62500;
    snd_pcm_uframes_t period_size = 64;
    snd_pcm_uframes_t buffer_size = 64 * 64;
    snd_pcm_hw_params_t *params;
    pcm_handle = nullptr;
    int err = snd_pcm_open(&pcm_handle, "plughw:0,0", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0)
        err = snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        fprintf(stderr, "[SND] snd_pcm_open failed: %s — sound disabled\n", snd_strerror(err));
        pcm_handle = nullptr;
        end = true;
        return;
    }
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
    if (pcm_handle) {
        snd_pcm_drain(pcm_handle);
        snd_pcm_close(pcm_handle);
    }
    delete[] stepBuffer;
    delete[] spinBuffer;
}

void SoundThread::run()
{
    if (!pcm_handle) return;
    QMutex mutex;
    mutex.lock();
    while (!end)
    {
        waitCondition.wait(&mutex);
        if (!enabled)
            memset(PSG::buffer, 0x80, PSG::bufferIndex);
        if (FDC::stepPulses > 0) {
            FDC::stepPulses = 0;
            if (sfxEnabled)
                stepPos = 0;
        }
        if (sfxEnabled && stepBuffer && stepPos < stepBufferLen) {
            int len = std::min(PSG::bufferIndex, stepBufferLen - stepPos);
            for (int i = 0; i < len; i++) {
                int mixed = (int)PSG::buffer[i] + (int)stepBuffer[stepPos + i] - 128;
                if (mixed < 0) mixed = 0;
                else if (mixed > 255) mixed = 255;
                PSG::buffer[i] = (BYTE)mixed;
            }
            stepPos += len;
        }
        if (sfxEnabled && spinBuffer && FDC::GetMotor()) {
            constexpr float spinGain = 0.2f;
            for (int i = 0; i < PSG::bufferIndex; i++) {
                int delta = (int)spinBuffer[spinPos] - 128;
                int mixed = (int)PSG::buffer[i] + (int)(delta * spinGain);
                if (mixed < 0) mixed = 0;
                else if (mixed > 255) mixed = 255;
                PSG::buffer[i] = (BYTE)mixed;
                spinPos++;
                if (spinPos >= spinBufferLen) spinPos = 0;
            }
        } else {
            spinPos = 0;
        }
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

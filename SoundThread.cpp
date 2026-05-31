#include "SoundThread.h"
#include "PSG.h"
#include "CPC.h"
#include <QMutex>
#include <QFile>
#include <QByteArray>
#include <cstring>
#include <cstdio>
#include <algorithm>
#ifndef _WIN32
#include <unistd.h>
#include <fcntl.h>
#endif

QWaitCondition SoundThread::waitCondition;
std::atomic<long> SoundThread::frames{0};
std::atomic<bool> SoundThread::enabled{true};
std::atomic<bool> SoundThread::sfxEnabled{true};

BYTE SoundThread::ringBuffer[RING_SIZE];
std::atomic<int> SoundThread::ringHead{0};
std::atomic<int> SoundThread::ringTail{0};
std::atomic<SoundThread *> SoundThread::instance{nullptr};

// Saturated-mix lookup table for two U8 PCM samples biased at 128.
// Uses Viktor T. Toth additive mixing (no hard clipping).
static BYTE g_mixTable[256][256];

static void buildMixTable()
{
    for (int a = 0; a < 256; a++)
    {
        const float fa = (a - 128) / 128.0f;
        for (int b = 0; b < 256; b++)
        {
            const float fb = (b - 128) / 128.0f;
            float s;
            if (fa >= 0.0f && fb >= 0.0f)      s = fa + fb - fa * fb;
            else if (fa < 0.0f && fb < 0.0f)   s = fa + fb + fa * fb;
            else                                s = fa + fb;
            int out = int(s * 128.0f + 128.5f);
            if (out < 0) out = 0;
            else if (out > 255) out = 255;
            g_mixTable[a][b] = BYTE(out);
        }
    }
}

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

SoundThread::SoundThread(QObject *parent) : QThread(parent), stream(nullptr), paInitialized(false)
{
    instance = this;
    buildMixTable();

    loadWavU8(":/images/step.wav", stepBuffer, stepBufferLen);
    stepPos = stepBufferLen;
    loadWavU8(":/images/spin.wav", spinBuffer, spinBufferLen);
    // Fold the spin gain into the spin samples so the hot loop can use the same mix table.
    // ((b - 128) * 51) >> 8 ≈ (b - 128) * 0.199
    if (spinBuffer)
    {
        for (int i = 0; i < spinBufferLen; i++)
            spinBuffer[i] = BYTE((((int(spinBuffer[i]) - 128) * 51) >> 8) + 128);
    }
    spinPos = 0;

#ifndef _WIN32
    int savedStderr = dup(STDERR_FILENO);
    int devNull = open("/dev/null", O_WRONLY);
    if (devNull >= 0) { fflush(stderr); dup2(devNull, STDERR_FILENO); close(devNull); }
#endif
    PaError err = Pa_Initialize();
#ifndef _WIN32
    if (savedStderr >= 0) { fflush(stderr); dup2(savedStderr, STDERR_FILENO); close(savedStderr); }
#endif
    if (err != paNoError) {
        fprintf(stderr, "[SND] Pa_Initialize failed: %s — sound disabled\n", Pa_GetErrorText(err));
        end = true;
        return;
    }
    paInitialized = true;

    PaStreamParameters outputParams;
#ifdef __linux__
    PaHostApiIndex alsaIdx = Pa_HostApiTypeIdToHostApiIndex(paALSA);
    outputParams.device = (alsaIdx >= 0)
        ? Pa_GetHostApiInfo(alsaIdx)->defaultOutputDevice
        : Pa_GetDefaultOutputDevice();
#else
    outputParams.device = Pa_GetDefaultOutputDevice();
#endif
    if (outputParams.device == paNoDevice) {
        fprintf(stderr, "[SND] No default output device — sound disabled\n");
        end = true;
        return;
    }
    outputParams.channelCount = 1;
    outputParams.sampleFormat = paUInt8;
    outputParams.suggestedLatency = Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;

    err = Pa_OpenStream(&stream, nullptr, &outputParams,
                        62500, 128, paNoFlag,
                        paCallback, nullptr);
    if (err != paNoError) {
        fprintf(stderr, "[SND] Pa_OpenStream failed: %s — sound disabled\n", Pa_GetErrorText(err));
        stream = nullptr;
        end = true;
        return;
    }
    end = false;
}

SoundThread::~SoundThread()
{
    instance = nullptr;
    if (stream) {
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
    }
    if (paInitialized)
        Pa_Terminate();
    delete[] stepBuffer;
    delete[] spinBuffer;
}

int SoundThread::paCallback(const void *input, void *output,
                            unsigned long frameCount,
                            const PaStreamCallbackTimeInfo *timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData)
{
    (void)input;
    (void)timeInfo;
    (void)statusFlags;
    (void)userData;

    BYTE *out = static_cast<BYTE *>(output);
    int tail = ringTail.load(std::memory_order_relaxed);
    int head = ringHead.load(std::memory_order_acquire);

    static BYTE lastSample = 128;
    for (unsigned long i = 0; i < frameCount; i++) {
        if (tail != head) {
            lastSample = ringBuffer[tail];
            out[i] = lastSample;
            tail = (tail + 1) & (RING_SIZE - 1);
        } else {
            out[i] = lastSample;
        }
    }
    ringTail.store(tail, std::memory_order_release);
    return paContinue;
}

void SoundThread::run()
{
    if (!stream) return;

    Pa_StartStream(stream);

    QMutex mutex;
    mutex.lock();
    while (!end)
        waitCondition.wait(&mutex, 200);
    mutex.unlock();

    Pa_StopStream(stream);
}

void SoundThread::pushFrame()
{
    if (!enabled)
        memset(CPC::psg.buffer, 0x80, CPC::psg.bufferIndex);
    if (sfxEnabled)
    {
        if (CPC::fdc.stepPulses > 0) {
            CPC::fdc.stepPulses = 0;
            stepPos = 0;
        }
        if (stepBuffer && stepPos < stepBufferLen) {
            int len = std::min(CPC::psg.bufferIndex, stepBufferLen - stepPos);
            for (int i = 0; i < len; i++)
                CPC::psg.buffer[i] = g_mixTable[CPC::psg.buffer[i]][stepBuffer[stepPos + i]];
            stepPos += len;
        }
        if (spinBuffer && CPC::fdc.GetMotor()) {
            const int n = CPC::psg.bufferIndex;
            for (int i = 0; i < n; i++) {
                CPC::psg.buffer[i] = g_mixTable[CPC::psg.buffer[i]][spinBuffer[spinPos]];
                if (++spinPos >= spinBufferLen) spinPos = 0;
            }
        } else {
            spinPos = 0;
        }
    }

    int head = ringHead.load(std::memory_order_relaxed);
    int tail = ringTail.load(std::memory_order_acquire);
    int space = (RING_SIZE - 1) - ((head - tail) & (RING_SIZE - 1));
    int count = std::min(CPC::psg.bufferIndex, space);
    for (int i = 0; i < count; i++) {
        ringBuffer[head] = CPC::psg.buffer[i];
        head = (head + 1) & (RING_SIZE - 1);
    }
    ringHead.store(head, std::memory_order_release);

    CPC::psg.bufferIndex = 0;
    frames = (head - ringTail.load(std::memory_order_acquire)) & (RING_SIZE - 1);
}

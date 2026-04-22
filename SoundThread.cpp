#include "SoundThread.h"
#include "PSG.h"
#include "CPC.h"
#include <QMutex>
#include <QFile>
#include <QByteArray>
#include <cstring>
#include <cstdio>
#include <algorithm>

QWaitCondition SoundThread::waitCondition;
std::atomic<long> SoundThread::frames{0};
std::atomic<bool> SoundThread::enabled{true};
std::atomic<bool> SoundThread::sfxEnabled{true};

BYTE SoundThread::ringBuffer[RING_SIZE];
std::atomic<int> SoundThread::ringHead{0};
std::atomic<int> SoundThread::ringTail{0};

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
    loadWavU8(":/images/step.wav", stepBuffer, stepBufferLen);
    stepPos = stepBufferLen;
    loadWavU8(":/images/spin.wav", spinBuffer, spinBufferLen);
    spinPos = 0;

    PaError err = Pa_Initialize();
    if (err != paNoError) {
        fprintf(stderr, "[SND] Pa_Initialize failed: %s — sound disabled\n", Pa_GetErrorText(err));
        end = true;
        return;
    }
    paInitialized = true;

    PaStreamParameters outputParams;
    outputParams.device = Pa_GetDefaultOutputDevice();
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

    for (unsigned long i = 0; i < frameCount; i++) {
        if (tail != head) {
            out[i] = ringBuffer[tail];
            tail = (tail + 1) & (RING_SIZE - 1);
        } else {
            out[i] = 128;
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
    {
        waitCondition.wait(&mutex);
        if (!enabled)
            memset(CPC::psg.buffer, 0x80, CPC::psg.bufferIndex);
        if (CPC::fdc.stepPulses > 0) {
            CPC::fdc.stepPulses = 0;
            if (sfxEnabled)
                stepPos = 0;
        }
        if (sfxEnabled && stepBuffer && stepPos < stepBufferLen) {
            int len = std::min(CPC::psg.bufferIndex, stepBufferLen - stepPos);
            for (int i = 0; i < len; i++) {
                int mixed = (int)CPC::psg.buffer[i] + (int)stepBuffer[stepPos + i] - 128;
                if (mixed < 0) mixed = 0;
                else if (mixed > 255) mixed = 255;
                CPC::psg.buffer[i] = (BYTE)mixed;
            }
            stepPos += len;
        }
        if (sfxEnabled && spinBuffer && CPC::fdc.GetMotor()) {
            constexpr float spinGain = 0.2f;
            for (int i = 0; i < CPC::psg.bufferIndex; i++) {
                int delta = (int)spinBuffer[spinPos] - 128;
                int mixed = (int)CPC::psg.buffer[i] + (int)(delta * spinGain);
                if (mixed < 0) mixed = 0;
                else if (mixed > 255) mixed = 255;
                CPC::psg.buffer[i] = (BYTE)mixed;
                spinPos++;
                if (spinPos >= spinBufferLen) spinPos = 0;
            }
        } else {
            spinPos = 0;
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

    Pa_StopStream(stream);
    mutex.unlock();
}

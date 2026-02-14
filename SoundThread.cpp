#include "SoundThread.h"
#include "Emulator/Headers/PSG.h"
#include "Emulator/Headers/CRTScreen.h"
#include <stdlib.h>
#include <asoundlib.h>

pa_threaded_mainloop *SoundThread::outputMainLoop ;
pa_context *SoundThread::outputContext;
pa_stream *SoundThread::outputStream;
volatile long SoundThread::lastElapsed;

using namespace std::chrono;
SoundThread::SoundThread(QObject *parent) : QThread(parent)
{
    end = false;
}

SoundThread::~SoundThread()
{

}

void SoundThread::run()
{
    unsigned int rate = 62500;
    unsigned int pcm;
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;

    pcm = snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(pcm_handle, params);
    pcm = snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_U8);
    pcm = snd_pcm_hw_params_set_channels(pcm_handle, params, 1);
    pcm = snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, 0);
    pcm = snd_pcm_hw_params(pcm_handle, params);
    if (snd_pcm_writei(pcm_handle, PSG::buffer, PSG::bufferIndex) == -EPIPE) {
        snd_pcm_prepare(pcm_handle);
    }
    long lastT = 0;
    long t;
    long elapsed;
    while (!end)
    {
        if (true) //running)
        {
            switch(CRTScreen::stage)
            {
            case CRTStage::WaitingEmulation:
                break;
            case CRTStage::Running:
                break;
            case CRTStage::WaitingAudio:
                if (snd_pcm_writei(pcm_handle, PSG::buffer, PSG::bufferIndex) == -EPIPE) {
                    snd_pcm_prepare(pcm_handle);
                }
                PSG::bufferIndex = 0;
                duration now = high_resolution_clock::now().time_since_epoch();
                t = duration_cast<microseconds>(now).count();
                elapsed = t - lastT;
                lastElapsed = elapsed;
                while (elapsed < 19968)
                {
                    duration now = high_resolution_clock::now().time_since_epoch();
                    t = duration_cast<microseconds>(now).count();
                    elapsed = t - lastT;
                };
                lastT = t;
                CRTScreen::stage = CRTStage::Running;
                break;
            }

        }
    }
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
}

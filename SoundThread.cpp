#include "SoundThread.h"
#include "Emulator/Headers/PSG.h"
#include "Emulator/Headers/CRTScreen.h"
#include <stdlib.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/mainloop.h>
#include <pulse/xmalloc.h>

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
    pa_simple_new();
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
                pa_simple_write(PSG::buffer, PSG::bufferIndex);
                PSG::bufferIndex = 0;
                duration now = high_resolution_clock::now().time_since_epoch();
                t = duration_cast<microseconds>(now).count();
                elapsed = t - lastT;
                lastElapsed = elapsed;
                while (elapsed < 20000)
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
    pa_simple_free();
}

void SoundThread::context_state_cb(pa_context *context, void *userData)
{
    pa_context_state_t state = pa_context_get_state(context);
    switch (state)
    {
    case PA_CONTEXT_READY:
    case PA_CONTEXT_TERMINATED:
    case PA_CONTEXT_FAILED:
        pa_threaded_mainloop_signal(outputMainLoop, 0);
        break;

    case PA_CONTEXT_UNCONNECTED:
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
        break;
    }
}

void SoundThread::pa_simple_new()
{
    outputMainLoop = pa_threaded_mainloop_new();
    outputContext = pa_context_new(pa_threaded_mainloop_get_api(outputMainLoop), "AAE");
    pa_context_set_state_callback(outputContext, context_state_cb, NULL);

    if (pa_context_connect(outputContext, NULL, PA_CONTEXT_NOFLAGS, NULL) < 0) {
        int error = pa_context_errno(outputContext);
    }


    pa_threaded_mainloop_lock(outputMainLoop);

    if (pa_threaded_mainloop_start(outputMainLoop) < 0)
    {
        //goto unlock_and_fail;
    }

    for (;;) {
        pa_context_state_t state;

        state = pa_context_get_state(outputContext);

        if (state == PA_CONTEXT_READY)
            break;

        if (!PA_CONTEXT_IS_GOOD(state)) {
            int error = pa_context_errno(outputContext);
            //goto unlock_and_fail;
        }

        /* Wait until the context is ready */
        pa_threaded_mainloop_wait(outputMainLoop);
    }

    pa_sample_spec *outputSpec = new(pa_sample_spec);
    outputSpec->format = PA_SAMPLE_U8;
    outputSpec->channels = 1;
    outputSpec->rate = 62500;

    if (!(outputStream = pa_stream_new(outputContext, "Output", outputSpec, NULL))) {
        int error = pa_context_errno(outputContext);
        //goto unlock_and_fail;
    }

         pa_stream_set_state_callback(outputStream, stream_state_cb, NULL);
         pa_stream_set_read_callback(outputStream, stream_request_cb, NULL);
         pa_stream_set_write_callback(outputStream, stream_request_cb, NULL);
         pa_stream_set_latency_update_callback(outputStream, stream_latency_update_cb, NULL);

         pa_stream_flags_t flags = (pa_stream_flags_t)
        (PA_STREAM_INTERPOLATE_TIMING
         |PA_STREAM_ADJUST_LATENCY
         |PA_STREAM_AUTO_TIMING_UPDATE);

    pa_stream_connect_playback(outputStream, NULL, NULL, flags, NULL, NULL);
    /*
                 r = pa_stream_connect_record(p->stream, dev, attr,
                                                                         PA_STREAM_INTERPOLATE_TIMING
                                                                             |PA_STREAM_ADJUST_LATENCY
                                                                             |PA_STREAM_AUTO_TIMING_UPDATE);
*/
    for (;;) {
        pa_stream_state_t state;

        state = pa_stream_get_state(outputStream);

        if (state == PA_STREAM_READY)
            break;

        if (!PA_STREAM_IS_GOOD(state)) {
            int error = pa_context_errno(outputContext);
            //goto unlock_and_fail;
        }

        /* Wait until the stream is ready */
        pa_threaded_mainloop_wait(outputMainLoop);
    }

    pa_threaded_mainloop_unlock(outputMainLoop);

}

int SoundThread::pa_simple_write(const void*data, size_t length) {

    pa_threaded_mainloop_lock(outputMainLoop);

//    CHECK_DEAD_GOTO(p, rerror, unlock_and_fail);

    while (length > 0) {
        size_t l;
        int r;

        while (!(l = pa_stream_writable_size(outputStream))) {
            pa_threaded_mainloop_wait(outputMainLoop);
            //CHECK_DEAD_GOTO(p, rerror, unlock_and_fail);
        }

        //CHECK_SUCCESS_GOTO(p, rerror, l != (size_t) -1, unlock_and_fail);

        if (l > length)
            l = length;

        r = pa_stream_write(outputStream, data, l, NULL, 0LL, PA_SEEK_RELATIVE);
        //CHECK_SUCCESS_GOTO(p, rerror, r >= 0, unlock_and_fail);

        data = (const uint8_t*) data + l;
        length -= l;
    }

    pa_threaded_mainloop_unlock(outputMainLoop);
    return 0;

unlock_and_fail:
    pa_threaded_mainloop_unlock(outputMainLoop);
    return -1;
}

void SoundThread::pa_simple_free() {
    if (outputMainLoop)
        pa_threaded_mainloop_stop(outputMainLoop);
    if (outputStream)
        pa_stream_unref(outputStream);
    if (outputContext) {
        pa_context_disconnect(outputContext);
        pa_context_unref(outputContext);
    }
    if (outputMainLoop)
        pa_threaded_mainloop_free(outputMainLoop);
    // pa_xfree(outputStream);
}

void SoundThread::stream_state_cb(pa_stream *s, void * userdata) {

    switch (pa_stream_get_state(s)) {

    case PA_STREAM_READY:
    case PA_STREAM_FAILED:
    case PA_STREAM_TERMINATED:
        pa_threaded_mainloop_signal(outputMainLoop, 0);
        break;
    case PA_STREAM_UNCONNECTED:
    case PA_STREAM_CREATING:
        break;
    }
}

void SoundThread::stream_request_cb(pa_stream *s, size_t length, void *userdata) {
    pa_threaded_mainloop_signal(outputMainLoop, 0);
}

void SoundThread::stream_latency_update_cb(pa_stream *s, void *userdata) {
    pa_threaded_mainloop_signal(outputMainLoop, 0);
}

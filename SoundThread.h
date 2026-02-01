#ifndef SOUNDTHREAD_H
#define SOUNDTHREAD_H

#include <QThread>
#include <pulse/context.h>
#include <pulse/thread-mainloop.h>
#include <pulse/stream.h>

class SoundThread : public QThread
{
    Q_OBJECT
public:
    explicit SoundThread(QObject *parent);
    ~SoundThread();
    volatile bool end;
signals:
protected:
    void run() override;
private:
    static void context_state_cb(pa_context *context, void *userData);
    static void stream_state_cb(pa_stream *s, void * userdata);
    static void stream_request_cb(pa_stream *s, size_t length, void *userdata);
    static void stream_latency_update_cb(pa_stream *s, void *userdata);
    void pa_simple_new();
    int pa_simple_write(const void *data, size_t length);
    void pa_simple_free();
    static pa_threaded_mainloop *outputMainLoop ;
    static pa_context *outputContext;
    static pa_stream *outputStream;
};

#endif // SOUNDTHREAD_H

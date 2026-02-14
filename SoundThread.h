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
    static volatile long lastElapsed;
signals:
protected:
    void run() override;
private:
    static pa_threaded_mainloop *outputMainLoop ;
    static pa_context *outputContext;
    static pa_stream *outputStream;
};

#endif // SOUNDTHREAD_H

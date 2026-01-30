#include "SoundThread.h"
#include "Emulator/Headers/CRTScreen.h"
#include <QFile>
#include <QFileDialog>
#include <QBuffer>
#include <QMediaDevices>
#include <QEventLoop>
#include <stdlib.h>

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
    /*
    QList devices = QMediaDevices::audioOutputs();
    QAudioDevice device = QAudioDevice(devices[0]);
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::UInt8);
*/
 //   volatile bool *dataAvailable = &CPC::psg->DataAvailable;

 //   QEventLoop loop;

    while (!end)
    {
        if (true) //running)
        {
            while (CRTScreen::newFrame == 0)
            {
                sleep(0);
            }
            CRTScreen::newFrame--;
            while (CRTScreen::newFrame > 0) {}
        }
 //       while (!*dataAvailable) {}
 //       *dataAvailable = false;
        /*
        output = new QAudioSink(device, format);
        QByteArray *bA = new QByteArray();
        for (int i = 0; i < 20000; i++)
        {
            while (!*dataAvailable) {}
            *dataAvailable = false;
            bA->append(random());  //CPC::psg->output
        }
        QBuffer *bufferA = new QBuffer(bA);
        bufferA->open(QIODevice::ReadOnly);
        output->start(bufferA);
        do {
            loop.processEvents();
        } while(output->state() == 0);
        delete output;
        delete bufferA;
        delete bA;
        */
    }

    //output->setBufferSize(32);
    //output->setVolume(1);
}

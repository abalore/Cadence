#include "mainwindow.h"
#include "KeyPressFilter.h"
#include <QApplication>
#include <QIcon>
#include <cstdlib>


int main(int argc, char *argv[])
{
    setenv("PIPEWIRE_LATENCY", "32/62500", 0);
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName(APP_NAME);
    QCoreApplication::setApplicationVersion(APP_VERSION);
    a.setWindowIcon(QIcon(":/images/cadence.svg"));
    MainWindow w;
    w.show();
    KeyPressFilter myKeyFilter;
    a.installEventFilter(&myKeyFilter);
    return a.exec();
}

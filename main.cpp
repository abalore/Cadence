#include "mainwindow.h"
#include "KeyPressFilter.h"
#include <QApplication>
#include <QIcon>


int main(int argc, char *argv[])
{
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

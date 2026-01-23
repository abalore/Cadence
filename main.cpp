#include "mainwindow.h"
#include "KeyPressFilter.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    KeyPressFilter myKeyFilter;
    a.installEventFilter(&myKeyFilter);
    return a.exec();
}

#include "mainwindow.h"
#include "KeyPressFilter.h"
#include <QApplication>
#include <QIcon>
#include <QPalette>
#include <QStyleFactory>
#include <cstdlib>


int main(int argc, char *argv[])
{
    setenv("PIPEWIRE_LATENCY", "32/62500", 0);
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName(APP_NAME);
    QCoreApplication::setApplicationVersion(APP_VERSION);

    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QPalette p;
    p.setColor(QPalette::Window,          QColor(53, 53, 53));
    p.setColor(QPalette::WindowText,      Qt::white);
    p.setColor(QPalette::Base,            QColor(35, 35, 35));
    p.setColor(QPalette::AlternateBase,   QColor(53, 53, 53));
    p.setColor(QPalette::Text,            Qt::white);
    p.setColor(QPalette::Button,          QColor(53, 53, 53));
    p.setColor(QPalette::ButtonText,      Qt::white);
    p.setColor(QPalette::Highlight,       QColor(42, 130, 218));
    p.setColor(QPalette::HighlightedText, Qt::black);
    p.setColor(QPalette::ToolTipBase,     Qt::white);
    p.setColor(QPalette::ToolTipText,     Qt::white);
    p.setColor(QPalette::PlaceholderText, QColor(160, 160, 160));
    p.setColor(QPalette::Link,            QColor(42, 130, 218));
    p.setColor(QPalette::Disabled, QPalette::Text,       QColor(127, 127, 127));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
    p.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
    QApplication::setPalette(p);

    a.setWindowIcon(QIcon(":/images/cadence.png"));
    MainWindow w;
    w.show();
    KeyPressFilter myKeyFilter;
    a.installEventFilter(&myKeyFilter);
    return a.exec();
}

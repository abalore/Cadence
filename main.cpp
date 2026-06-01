#include "mainwindow.h"
#include "KeyPressFilter.h"
#include <QApplication>
#include <QIcon>
#include <QPalette>
#include <QStyleFactory>
#include <QProxyStyle>
#include <QStyleOption>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include "Settings.h"
#include <cstdlib>

static void setupAppDataDir()
{
    QString newDir = Settings::CadenceDir();
    if (QDir(newDir).exists()) return;

    QString oldDir = QDir::homePath() + "/.cadence";
    if (QDir(oldDir).exists())
    {
        QDir().mkpath(QFileInfo(newDir).absolutePath());
        QDir().rename(oldDir, newDir);
        return;
    }

    for (const char *sub : {"BIN", "CDT", "CPR", "DSK", "ROM"})
        QDir().mkpath(newDir + "/" + sub);

    QString romDst = newDir + "/ROM";
    for (const QString &name : QDir(":/data/ROM").entryList(QDir::Files))
    {
        QString out = romDst + "/" + name;
        if (QFile::exists(out)) continue;
        if (QFile::copy(":/data/ROM/" + name, out))
            QFile::setPermissions(out, QFile::ReadOwner | QFile::WriteOwner);
    }
}

class MenuIndicatorStyle : public QProxyStyle
{
public:
    using QProxyStyle::QProxyStyle;
    void drawPrimitive(PrimitiveElement el, const QStyleOption *opt,
                       QPainter *p, const QWidget *w = nullptr) const override
    {
        const bool isMenu = qobject_cast<const QMenu *>(w);
        if (isMenu && (el == PE_IndicatorCheckBox || el == PE_IndicatorRadioButton))
        {
            QRect r = opt->rect.adjusted(1, 1, -1, -1);
            const QColor outline(0xa0, 0xa0, 0xa0);
            const QColor accent(0x00, 0xe5, 0xff);
            p->save();
            p->setRenderHint(QPainter::Antialiasing, true);
            p->setPen(QPen(outline, 1));
            p->setBrush(Qt::NoBrush);
            if (el == PE_IndicatorRadioButton)
            {
                p->drawEllipse(r);
                if (opt->state & State_On)
                {
                    p->setBrush(accent);
                    p->setPen(Qt::NoPen);
                    p->drawEllipse(r.adjusted(3, 3, -3, -3));
                }
            }
            else
            {
                p->drawRect(r);
                if (opt->state & State_On)
                {
                    QPainterPath path;
                    qreal x = r.x(), y = r.y(), ww = r.width(), hh = r.height();
                    path.moveTo(x + ww * 0.22, y + hh * 0.52);
                    path.lineTo(x + ww * 0.44, y + hh * 0.74);
                    path.lineTo(x + ww * 0.80, y + hh * 0.28);
                    p->setPen(QPen(accent, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    p->drawPath(path);
                }
            }
            p->restore();
            return;
        }
        QProxyStyle::drawPrimitive(el, opt, p, w);
    }
};


int main(int argc, char *argv[])
{
#ifndef _WIN32
    // PipeWire/JACK only — ignored by the WASAPI/WMME backend on Windows,
    // and setenv() isn't in the MinGW CRT.
    setenv("PIPEWIRE_LATENCY", "32/62500", 0);
    setenv("JACK_NO_START_SERVER", "1", 0);
#endif
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName(APP_NAME);
    QCoreApplication::setApplicationVersion(APP_VERSION);
    setupAppDataDir();

    QApplication::setStyle(new MenuIndicatorStyle(QStyleFactory::create("Fusion")));
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

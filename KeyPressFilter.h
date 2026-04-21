#ifndef KEYPRESSFILTER_H
#define KEYPRESSFILTER_H

#include "CPC.h"
#include "mainwindow.h"
#include <QObject>
#include <QKeyEvent>
#include <QApplication>

class KeyPressFilter : public QObject {
public:
    bool eventFilter(QObject* aObject, QEvent* aEvent) final
    {
        if (!qobject_cast<MainWindow*>(QApplication::activeWindow()))
            return QObject::eventFilter(aObject, aEvent);
        if (aEvent->type() == QEvent::KeyPress)
        {
            QKeyEvent *ke = static_cast<QKeyEvent*>(aEvent);
            if (!ke->isAutoRepeat())
#ifdef __APPLE__
                CPC::keyboard.KeyEvent(ke->nativeVirtualKey(), false);
#else
                CPC::keyboard.KeyEvent(ke->nativeScanCode(), false);
#endif
            return true;
        }
        else if (aEvent->type() == QEvent::KeyRelease)
        {
            QKeyEvent *ke = static_cast<QKeyEvent*>(aEvent);
            if (!ke->isAutoRepeat())
#ifdef __APPLE__
                CPC::keyboard.KeyEvent(ke->nativeVirtualKey(), true);
#else
                CPC::keyboard.KeyEvent(ke->nativeScanCode(), true);
#endif
            return true;
        }
        return QObject::eventFilter(aObject, aEvent);
    }
};

#endif // KEYPRESSFILTER_H

#ifndef KEYPRESSFILTER_H
#define KEYPRESSFILTER_H

#include "Keyboard.h"
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
                Keyboard::KeyEvent(ke->nativeScanCode(), false);
            return true;
        }
        else if (aEvent->type() == QEvent::KeyRelease)
        {
            QKeyEvent *ke = static_cast<QKeyEvent*>(aEvent);
            if (!ke->isAutoRepeat())
                Keyboard::KeyEvent(ke->nativeScanCode(), true);
            return true;
        }
        return QObject::eventFilter(aObject, aEvent);
    }
};

#endif // KEYPRESSFILTER_H

#ifndef KEYPRESSFILTER_H
#define KEYPRESSFILTER_H

#include "Keyboard.h"
#include <QObject>
#include <QKeyEvent>

class KeyPressFilter : public QObject {
public:
    bool eventFilter(QObject* aObject, QEvent* aEvent) final
    {
        if (aEvent->type() == QEvent::KeyPress)
        {
            QKeyEvent *ke = static_cast<QKeyEvent*>(aEvent);
            if (!ke->isAutoRepeat())
                Keyboard::KeyEvent(ke->nativeScanCode(), false);
        }
        else if (aEvent->type() == QEvent::KeyRelease)
        {
            QKeyEvent *ke = static_cast<QKeyEvent*>(aEvent);
            if (!ke->isAutoRepeat())
                Keyboard::KeyEvent(ke->nativeScanCode(), true);
        }
        return QObject::eventFilter(aObject, aEvent);
    }
};

#endif // KEYPRESSFILTER_H

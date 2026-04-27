#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include "Settings.h"

class QCheckBox;
class QRadioButton;
class QComboBox;

class PreferencesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PreferencesDialog(const Settings &current, bool unlockSpeed, QWidget *parent = nullptr);
    Settings result() const;
    bool unlockSpeed() const;

private:
    void openROMBox();

    QCheckBox *smoothCheck;
    QCheckBox *greenMonitorCheck;
    QCheckBox *fullScreenCheck;
    QComboBox *persistenceCombo;

    QCheckBox *audioEnabledCheck;
    QCheckBox *sfxEnabledCheck;
    QCheckBox *tapeAudioCheck;

    QCheckBox *rsBackslashCheck;
    QCheckBox *joystickCheck;

    QRadioButton *crtcRadios[5];
    QCheckBox *ram512kCheck;

    QCheckBox *unlockSpeedCheck;

    Settings settingsCopy;
};

#endif // PREFERENCESDIALOG_H

#include "Settings.h"
#include <QSettings>
#include <QDir>

QString Settings::ConfigDir()
{
    return QDir::homePath() + "/.config/cadence";
}

QString Settings::ConfigFile()
{
    return ConfigDir() + "/settings.cfg";
}

void Settings::Load()
{
    QSettings s(ConfigFile(), QSettings::IniFormat);
    smooth        = s.value("screen/smooth", true).toBool();
    greenMonitor  = s.value("screen/green_monitor", false).toBool();
    fullScreen    = s.value("screen/full_screen", false).toBool();
    audioEnabled  = s.value("audio/enabled", true).toBool();
    sfxEnabled    = s.value("audio/sfx", true).toBool();
    tapeEnabled   = s.value("audio/tape", true).toBool();
    rsBackslash   = s.value("keyboard/right_shift_as_backslash", true).toBool();
    diskAPath     = s.value("media/disk_a").toString();
    diskBPath     = s.value("media/disk_b").toString();
    tapePath      = s.value("media/tape").toString();
    cartridgePath = s.value("media/cartridge").toString();
}

void Settings::Save()
{
    QDir().mkpath(ConfigDir());
    QSettings s(ConfigFile(), QSettings::IniFormat);
    s.setValue("screen/smooth", smooth);
    s.setValue("screen/green_monitor", greenMonitor);
    s.setValue("screen/full_screen", fullScreen);
    s.setValue("audio/enabled", audioEnabled);
    s.setValue("audio/sfx", sfxEnabled);
    s.setValue("audio/tape", tapeEnabled);
    s.setValue("keyboard/right_shift_as_backslash", rsBackslash);
    s.setValue("media/disk_a", diskAPath);
    s.setValue("media/disk_b", diskBPath);
    s.setValue("media/tape", tapePath);
    s.setValue("media/cartridge", cartridgePath);
}

#include "Settings.h"
#include <QSettings>
#include <QDir>
#include <QStringList>

QString Settings::ConfigDir()
{
    return QDir::homePath() + "/.cadence";
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
    crtcType      = s.value("machine/crtc_type", 0).toInt();
    ram512kExpansion = s.value("memory/ram_512k_expansion", false).toBool();
    breakpointsEnabled = s.value("debugger/breakpoints_enabled", true).toBool();
    joystickEmulation = s.value("input/joystick_emulation", false).toBool();
    breakpoints.clear();
    QString bps = s.value("debugger/breakpoints").toString();
    for (const QString &t : bps.split(',', Qt::SkipEmptyParts))
    {
        bool ok;
        int a = t.toInt(&ok, 16);
        if (ok && a >= 0 && a <= 0xFFFF) breakpoints.append(a);
    }
    phosphorPersistence = s.value("screen/phosphor_persistence", 0).toInt();
    system        = s.value("machine/system", "CPC6128").toString();
    diskAPath     = s.value("media/disk_a").toString();
    diskBPath     = s.value("media/disk_b").toString();
    tapePath      = s.value("media/tape").toString();
    cartridgePath = s.value("media/cartridge").toString();
    for (int i = 0; i < 16; i++)
        romPaths[i] = s.value(QString("rom/slot_%1").arg(i)).toString();
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
    s.setValue("machine/crtc_type", crtcType);
    s.setValue("memory/ram_512k_expansion", ram512kExpansion);
    s.setValue("debugger/breakpoints_enabled", breakpointsEnabled);
    s.setValue("input/joystick_emulation", joystickEmulation);
    QStringList bpStrs;
    for (int a : breakpoints) bpStrs.append(QString::number(a, 16).toUpper());
    s.setValue("debugger/breakpoints", bpStrs.join(','));
    s.setValue("screen/phosphor_persistence", phosphorPersistence);
    s.setValue("machine/system", system);
    s.setValue("media/disk_a", diskAPath);
    s.setValue("media/disk_b", diskBPath);
    s.setValue("media/tape", tapePath);
    s.setValue("media/cartridge", cartridgePath);
    for (int i = 0; i < 16; i++)
        s.setValue(QString("rom/slot_%1").arg(i), romPaths[i]);
}

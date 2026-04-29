#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
#include <QVector>
#include <QMap>

class Settings
{
public:
    bool smooth = true;
    bool greenMonitor = false;
    bool fullScreen = false;
    bool audioEnabled = true;
    bool sfxEnabled = true;
    bool tapeEnabled = true;
    bool rsBackslash = true;
    int crtcType = 0;
    bool ram512kExpansion = false;
    bool breakpointsEnabled = true;
    bool joystickEmulation = false;
    QVector<int> breakpoints;
    QMap<int, QString> breakpointConditions;
    int phosphorPersistence = 0;
    QString system = "CPC6128";
    QString diskAPath;
    QString diskBPath;
    QString tapePath;
    QString cartridgePath;
    QString tapeDir;
    QString diskDir;
    QString cartridgeDir;
    QString assemblerDir;
    QString romPaths[16];

    void Load();
    void Save();

    static QString CadenceDir();

private:
    static QString ConfigDir();
    static QString ConfigFile();
};

#endif // SETTINGS_H

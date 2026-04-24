#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>

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
    int phosphorPersistence = 0;
    QString system = "CPC6128";
    QString diskAPath;
    QString diskBPath;
    QString tapePath;
    QString cartridgePath;
    QString romPaths[16];

    void Load();
    void Save();

private:
    static QString ConfigDir();
    static QString ConfigFile();
};

#endif // SETTINGS_H

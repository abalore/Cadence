#ifndef ROMBOXDIALOG_H
#define ROMBOXDIALOG_H

#include <QDialog>
#include <QString>

class QTableWidget;
class Settings;

class ROMBoxDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ROMBoxDialog(Settings *settings, QWidget *parent = nullptr);

private slots:
    void loadSlot(int slot);
    void clearSlot(int slot);

private:
    void refreshRow(int slot);
    QString slotLabel(int slot) const;

    Settings *settings;
    QTableWidget *table;
};

#endif // ROMBOXDIALOG_H

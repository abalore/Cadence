#ifndef ENTERBYTESDIALOG_H
#define ENTERBYTESDIALOG_H

#include <QDialog>

namespace Ui {
class EnterBytesDialog;
}

class EnterBytesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EnterBytesDialog(QWidget *parent = nullptr);
    ~EnterBytesDialog();

private:
    Ui::EnterBytesDialog *ui;

    void onAccepted();
};

#endif // ENTERBYTESDIALOG_H

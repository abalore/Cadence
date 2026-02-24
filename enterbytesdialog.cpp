#include "enterbytesdialog.h"
#include "ui_enterbytesdialog.h"
#include "Emulator/Headers/CPC.h"

EnterBytesDialog::EnterBytesDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EnterBytesDialog)
{
    ui->setupUi(this);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &EnterBytesDialog::onAccepted);
}

EnterBytesDialog::~EnterBytesDialog()
{
    delete ui;
}

void EnterBytesDialog::onAccepted()
{
    unsigned short address = ui->inputAddress->text().toUShort(nullptr, 16);
    QStringList values = ui->inputBytes->toPlainText().split(',');
    for (int i = 0; i < values.count(); i++)
        CPC::SetByteAt(address++, values[i].toUShort(nullptr, 16));
}

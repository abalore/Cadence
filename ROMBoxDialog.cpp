#include "ROMBoxDialog.h"
#include "Settings.h"
#include "CPC.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QLabel>

ROMBoxDialog::ROMBoxDialog(Settings *settings, QWidget *parent)
    : QDialog(parent), settings(settings)
{
    setWindowTitle(tr("ROM Box"));
    resize(560, 480);

    table = new QTableWidget(16, 4, this);
    QStringList headers;
    headers << tr("Slot") << tr("ROM") << QString() << QString();
    table->setHorizontalHeaderLabels(headers);
    table->verticalHeader()->setVisible(false);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setFocusPolicy(Qt::NoFocus);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    for (int i = 0; i < 16; i++)
    {
        QTableWidgetItem *slotItem = new QTableWidgetItem(QString::number(i));
        slotItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 0, slotItem);
        table->setItem(i, 1, new QTableWidgetItem());

        QPushButton *loadBtn = new QPushButton(tr("Load..."), table);
        connect(loadBtn, &QPushButton::clicked, this, [this, i]{ loadSlot(i); });
        table->setCellWidget(i, 2, loadBtn);

        QPushButton *clearBtn = new QPushButton(tr("Clear"), table);
        connect(clearBtn, &QPushButton::clicked, this, [this, i]{ clearSlot(i); });
        table->setCellWidget(i, 3, clearBtn);

        refreshRow(i);
    }

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::accept);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(table);
    layout->addWidget(buttons);
}

QString ROMBoxDialog::slotLabel(int slot) const
{
    const QString &path = settings->romPaths[slot];
    if (!path.isEmpty())
        return QFileInfo(path).fileName() + "  [" + path + "]";

    switch (CPC::cpcType)
    {
    case CPCType::CPC464:
        if (slot == 0) return tr("(built-in) BASIC 464");
        break;
    case CPCType::CPC664:
        if (slot == 0) return tr("(built-in) BASIC 664");
        if (slot == 7) return tr("(built-in) AMSDOS");
        break;
    case CPCType::CPC6128:
        if (slot == 0) return tr("(built-in) BASIC 6128");
        if (slot == 7) return tr("(built-in) AMSDOS");
        break;
    }
    return tr("<empty>");
}

void ROMBoxDialog::refreshRow(int slot)
{
    QTableWidgetItem *item = table->item(slot, 1);
    if (item) item->setText(slotLabel(slot));
}

void ROMBoxDialog::loadSlot(int slot)
{
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Load ROM into slot %1").arg(slot), QDir::homePath() + "/.cadence/ROM",
        tr("ROM Files (*.bin *.rom *.BIN *.ROM)"), nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty()) return;
    CPC::ReadROM((char *)fileName.toUtf8().data(), slot);
    settings->romPaths[slot] = fileName;
    refreshRow(slot);
}

void ROMBoxDialog::clearSlot(int slot)
{
    settings->romPaths[slot].clear();
    CPC::ClearROM(slot);
    refreshRow(slot);
}

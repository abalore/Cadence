#include "AboutDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QIcon>

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setStyleSheet(
        "QDialog { background-color: #0b1220; border: 1px solid #00e5ff; }"
        "QLabel { color: #e0e8f0; background: transparent; }"
        "QPushButton { background-color: #0b1220; color: #00e5ff; "
        "border: 1px solid #00e5ff; padding: 6px 28px; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #00e5ff; color: #0b1220; }"
        "QPushButton:pressed { background-color: #00b8cc; color: #0b1220; }"
    );

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(40, 28, 40, 24);
    layout->setSpacing(10);

    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(QIcon(":/images/cadence.png").pixmap(80, 80));
    iconLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(iconLabel, 0, Qt::AlignHCenter);

    QLabel *titleLabel = new QLabel(QString("%1 %2").arg(APP_NAME, APP_VERSION), this);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 5);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    QLabel *copyLabel = new QLabel("(c) Abalore 2026", this);
    QFont copyFont = copyLabel->font();
    copyFont.setPointSize(copyFont.pointSize() - 1);
    copyLabel->setFont(copyFont);
    copyLabel->setAlignment(Qt::AlignCenter);
    copyLabel->setStyleSheet("QLabel { color: #8a96a8; }");
    layout->addWidget(copyLabel);

    layout->addSpacing(12);

    QPushButton *okButton = new QPushButton("OK", this);
    okButton->setDefault(true);
    okButton->setFocusPolicy(Qt::StrongFocus);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(okButton, 0, Qt::AlignHCenter);
}

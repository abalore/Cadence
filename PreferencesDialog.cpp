#include "PreferencesDialog.h"
#include "ROMBoxDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QButtonGroup>

PreferencesDialog::PreferencesDialog(const Settings &current, bool unlockSpd, QWidget *parent)
    : QDialog(parent), settingsCopy(current)
{
    setWindowTitle(tr("Preferences"));
    setStyleSheet(
        "QDialog { background-color: #0b1220; }"
        "QGroupBox { color: #e0e8f0; border: 1px solid #505050; border-radius: 4px; "
        "  margin-top: 14px; padding: 10px 8px 8px 8px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; }"
        "QCheckBox, QRadioButton, QLabel { color: #e0e8f0; }"
        "QComboBox { background-color: #1a2535; color: #e0e8f0; border: 1px solid #505050; "
        "  border-radius: 3px; padding: 3px 8px; }"
        "QComboBox::drop-down { border: none; }"
        "QComboBox QAbstractItemView { background-color: #1a2535; color: #e0e8f0; "
        "  selection-background-color: #2a82da; }"
        "QPushButton { background-color: #0b1220; color: #b6b6b6; "
        "  border: 1px solid #b6b6b6; padding: 6px 16px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #b6b6b6; color: #0b1220; }"
        "QPushButton:pressed { background-color: #929292; color: #0b1220; }"
    );

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(6);
    mainLayout->setContentsMargins(16, 12, 16, 12);

    auto *columns = new QHBoxLayout;
    columns->setSpacing(12);
    auto *leftColumn  = new QVBoxLayout;
    auto *rightColumn = new QVBoxLayout;
    leftColumn->setSpacing(6);
    rightColumn->setSpacing(6);

    // --- Screen ---
    auto *screenGroup = new QGroupBox(tr("Screen"));
    auto *screenLayout = new QVBoxLayout(screenGroup);
    smoothCheck = new QCheckBox(tr("Smooth"), screenGroup);
    greenMonitorCheck = new QCheckBox(tr("Green monitor"), screenGroup);
    fullScreenCheck = new QCheckBox(tr("Full screen"), screenGroup);
    screenLayout->addWidget(smoothCheck);
    screenLayout->addWidget(greenMonitorCheck);
    screenLayout->addWidget(fullScreenCheck);

    auto *persRow = new QHBoxLayout;
    persRow->addWidget(new QLabel(tr("Phosphor persistence:"), screenGroup));
    persistenceCombo = new QComboBox(screenGroup);
    persistenceCombo->addItem(tr("Off"));
    for (int i = 1; i <= 5; i++)
        persistenceCombo->addItem(tr("%1 frame%2").arg(i).arg(i > 1 ? "s" : ""));
    persRow->addWidget(persistenceCombo);
    persRow->addStretch();
    screenLayout->addLayout(persRow);
    rightColumn->addWidget(screenGroup);

    // --- Audio ---
    auto *audioGroup = new QGroupBox(tr("Audio"));
    auto *audioLayout = new QVBoxLayout(audioGroup);
    audioEnabledCheck = new QCheckBox(tr("Audio enabled"), audioGroup);
    sfxEnabledCheck = new QCheckBox(tr("SFX"), audioGroup);
    tapeAudioCheck = new QCheckBox(tr("Tape"), audioGroup);
    audioLayout->addWidget(audioEnabledCheck);
    audioLayout->addWidget(sfxEnabledCheck);
    audioLayout->addWidget(tapeAudioCheck);
    auto *latRow = new QHBoxLayout;
    latRow->addWidget(new QLabel(tr("Latency:"), audioGroup));
    audioLatencyCombo = new QComboBox(audioGroup);
    audioLatencyCombo->addItem(tr("Low (128)"), 128);
    audioLatencyCombo->addItem(tr("Normal (256)"), 256);
    audioLatencyCombo->addItem(tr("Safe (512)"), 512);
    latRow->addWidget(audioLatencyCombo);
    latRow->addStretch();
    audioLayout->addLayout(latRow);
    auto *latNote = new QLabel(tr("(applies on restart)"), audioGroup);
    latNote->setStyleSheet("color: #8a93a0; font-size: 11px;");
    audioLayout->addWidget(latNote);
    rightColumn->addWidget(audioGroup);

    // --- Input ---
    auto *inputGroup = new QGroupBox(tr("Input"));
    auto *inputLayout = new QVBoxLayout(inputGroup);
    rsBackslashCheck = new QCheckBox(tr("Right shift as \\"), inputGroup);
    joystickCheck = new QCheckBox(tr("Joystick emulation"), inputGroup);
    inputLayout->addWidget(rsBackslashCheck);
    inputLayout->addWidget(joystickCheck);
    leftColumn->addWidget(inputGroup);

    // --- Hardware ---
    auto *hwGroup = new QGroupBox(tr("Hardware"));
    auto *hwLayout = new QVBoxLayout(hwGroup);

    hwLayout->addWidget(new QLabel(tr("CRTC type:"), hwGroup));
    auto *crtcBtnGroup = new QButtonGroup(this);
    static const char *crtcLabels[5] = {
        "0 - HD6845S", "1 - UM6845R", "2 - MC6845", "3 - Pre-ASIC", "4 - ASIC"
    };
    for (int i = 0; i < 5; i++)
    {
        crtcRadios[i] = new QRadioButton(tr(crtcLabels[i]), hwGroup);
        crtcBtnGroup->addButton(crtcRadios[i], i);
        hwLayout->addWidget(crtcRadios[i]);
    }

    ram512kCheck = new QCheckBox(tr("512k RAM expansion"), hwGroup);
    hwLayout->addWidget(ram512kCheck);

    auto *romBoxBtn = new QPushButton(tr("ROM Box..."), hwGroup);
    connect(romBoxBtn, &QPushButton::clicked, this, &PreferencesDialog::openROMBox);
    hwLayout->addWidget(romBoxBtn, 0, Qt::AlignLeft);
    leftColumn->addWidget(hwGroup);

    // --- Performance ---
    auto *perfGroup = new QGroupBox(tr("Performance"));
    auto *perfLayout = new QVBoxLayout(perfGroup);
    unlockSpeedCheck = new QCheckBox(tr("Unlock speed"), perfGroup);
    perfLayout->addWidget(unlockSpeedCheck);
    rightColumn->addWidget(perfGroup);

    leftColumn->addStretch();
    rightColumn->addStretch();
    columns->addLayout(leftColumn);
    columns->addLayout(rightColumn);
    mainLayout->addLayout(columns);

    // --- Buttons ---
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->setStyleSheet(
        "QDialogButtonBox QPushButton { min-width: 80px; }");
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttons);

    // --- Populate from current settings ---
    smoothCheck->setChecked(current.smooth);
    greenMonitorCheck->setChecked(current.greenMonitor);
    fullScreenCheck->setChecked(current.fullScreen);
    int pLevel = current.phosphorPersistence;
    persistenceCombo->setCurrentIndex((pLevel >= 0 && pLevel <= 5) ? pLevel : 0);
    audioEnabledCheck->setChecked(current.audioEnabled);
    sfxEnabledCheck->setChecked(current.sfxEnabled);
    tapeAudioCheck->setChecked(current.tapeEnabled);
    int latIdx = audioLatencyCombo->findData(current.audioLatencyFrames);
    audioLatencyCombo->setCurrentIndex(latIdx >= 0 ? latIdx : 1);
    rsBackslashCheck->setChecked(current.rsBackslash);
    joystickCheck->setChecked(current.joystickEmulation);
    int ct = (current.crtcType >= 0 && current.crtcType < 5) ? current.crtcType : 0;
    crtcRadios[ct]->setChecked(true);
    ram512kCheck->setChecked(current.ram512kExpansion);
    unlockSpeedCheck->setChecked(unlockSpd);
}

Settings PreferencesDialog::result() const
{
    Settings s = settingsCopy;
    s.smooth = smoothCheck->isChecked();
    s.greenMonitor = greenMonitorCheck->isChecked();
    s.fullScreen = fullScreenCheck->isChecked();
    s.phosphorPersistence = persistenceCombo->currentIndex();
    s.audioEnabled = audioEnabledCheck->isChecked();
    s.sfxEnabled = sfxEnabledCheck->isChecked();
    s.tapeEnabled = tapeAudioCheck->isChecked();
    s.audioLatencyFrames = audioLatencyCombo->currentData().toInt();
    s.rsBackslash = rsBackslashCheck->isChecked();
    s.joystickEmulation = joystickCheck->isChecked();
    for (int i = 0; i < 5; i++)
        if (crtcRadios[i]->isChecked()) { s.crtcType = i; break; }
    s.ram512kExpansion = ram512kCheck->isChecked();
    return s;
}

bool PreferencesDialog::unlockSpeed() const
{
    return unlockSpeedCheck->isChecked();
}

void PreferencesDialog::openROMBox()
{
    ROMBoxDialog dlg(&settingsCopy, this);
    dlg.exec();
}

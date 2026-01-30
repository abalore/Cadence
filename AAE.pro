QT += core
QT += gui
QT += multimedia


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += qt

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Debugger.cpp \
    Emulator/CRTScreen.cpp \
    Emulator/Keyboard.cpp \
    Emulator/PPI.cpp \
    Emulator/PSG.cpp \
    Emulator/ROMSelector.cpp \
    Emulator/Z80_IDX_3.cpp \
    Emulator/Z80_intexec.cpp \
    EmulatorWorkerThread.cpp \
    SoundThread.cpp \
    main.cpp \
    mainwindow.cpp \
    Emulator/CPC.cpp \
    Emulator/CRTC.cpp \
    Emulator/Disassembler.cpp \
    Emulator/Emulator.cpp \
    Emulator/Flag.cpp \
    Emulator/GateArray.cpp \
    Emulator/RAM.cpp \
    Emulator/ROM.cpp \
    Emulator/Reg16.cpp \
    Emulator/Z80.cpp \
    Emulator/Z80_CB.cpp \
    Emulator/Z80_IDX.cpp \
    Emulator/Z80_IDX_2.cpp \
    Emulator/Z80_IDX_CB.cpp \
    Emulator/Z80_Instr.cpp \
    Emulator/Z80_basic.cpp \
    Emulator/Z80_misc.cpp

HEADERS += \
    Debugger.h \
    Emulator/Headers/CPC.h \
    Emulator/Headers/CRTC.h \
    Emulator/Headers/CRTScreen.h \
    Emulator/Headers/Disassembler.h \
    Emulator/Headers/Emulator.h \
    Emulator/Headers/Flag.h \
    Emulator/Headers/GateArray.h \
    Emulator/Headers/Keyboard.h \
    Emulator/Headers/PPI.h \
    Emulator/Headers/PSG.h \
    Emulator/Headers/RAM.h \
    Emulator/Headers/ROM.h \
    Emulator/Headers/ROMSelector.h \
    Emulator/Headers/Reg16.h \
    Emulator/Headers/Z80.h \
    Emulator/Headers/defs.h \
    EmulatorWorkerThread.h \
    KeyPressFilter.h \
    SoundThread.h \
    mainwindow.h

FORMS += \
    Debugger.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



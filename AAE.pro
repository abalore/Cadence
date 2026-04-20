QT += core
QT += gui
QT += opengl
QT += openglwidgets

VERSION = 0.1
TARGET = cadence
DEFINES += APP_VERSION=\\\"0.1a\\\" APP_NAME=\\\"Cadence\\\"

QMAKE_CXXFLAGS+=-Wall
QMAKE_CXXFLAGS+=-Werror
#QMAKE_CXXFLAGS+=-fsanitize=address,leak
#QMAKE_LFLAGS+=-fsanitize=address,leak

LIBS += -lasound

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += qt

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Debugger.cpp \
    CRTScreen.cpp \
    DSK.cpp \
    FDC.cpp \
    FloppyDrive.cpp \
    Keyboard.cpp \
    PPI.cpp \
    PSG.cpp \
    Tape.cpp \
    Z80_16bitAL.cpp \
    Z80_8bitAL.cpp \
    Z80_8bitShift.cpp \
    Z80_IDX_3.cpp \
    Z80_Jump.cpp \
    Z80_Load.cpp \
    Z80_intexec.cpp \
    EmulatorThread.cpp \
    SoundThread.cpp \
    enterbytesdialog.cpp \
    graphicsinspector.cpp \
    main.cpp \
    mainwindow.cpp \
    CDT.cpp \
    CPC.cpp \
    CRTC.cpp \
    Disassembler.cpp \
    Emulator.cpp \
    GateArray.cpp \
    Reg16.cpp \
    Z80.cpp \
    Z80_CB.cpp \
    Z80_IDX.cpp \
    Z80_IDX_2.cpp \
    Z80_IDX_CB.cpp \
    Z80_Instr.cpp \
    Z80_basic.cpp \
    Z80_misc.cpp \
    pboWidget.cpp \
    pboWidget.h \
    speedcontroller.cpp

HEADERS += \
    Debugger.h \
    CDT.h \
    CPC.h \
    CRTC.h \
    CRTScreen.h \
    DSK.h \
    Disassembler.h \
    Emulator.h \
    FDC.h \
    FloppyDrive.h \
    GateArray.h \
    Keyboard.h \
    PPI.h \
    PSG.h \
    Reg16.h \
    Tape.h \
    Z80.h \
    defs.h \
    EmulatorThread.h \
    KeyPressFilter.h \
    SoundThread.h \
    enterbytesdialog.h \
    graphicsinspector.h \
    mainwindow.h \
    speedcontroller.h

FORMS += \
    Debugger.ui \
    enterbytesdialog.ui \
    graphicsinspector.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
  screen_frame.qrc


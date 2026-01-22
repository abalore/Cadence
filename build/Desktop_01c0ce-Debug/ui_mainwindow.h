/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QGraphicsView *graphicsView;
    QPushButton *btnStepIn;
    QLabel *lblZ80;
    QLabel *lblStack;
    QLabel *lblDisassembler;
    QPushButton *btnStepOut;
    QPushButton *btnStepOver;
    QLabel *lblCRTC;
    QLabel *lblGateArray;
    QPushButton *btnRender;
    QMenuBar *menubar;
    QMenu *menuHello;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->setWindowModality(Qt::NonModal);
        MainWindow->resize(1565, 727);
        MainWindow->setAutoFillBackground(false);
        MainWindow->setStyleSheet(QString::fromUtf8(""));
        MainWindow->setDocumentMode(false);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        graphicsView = new QGraphicsView(centralwidget);
        graphicsView->setObjectName("graphicsView");
        graphicsView->setGeometry(QRect(10, 10, 1061, 680));
        btnStepIn = new QPushButton(centralwidget);
        btnStepIn->setObjectName("btnStepIn");
        btnStepIn->setGeometry(QRect(1180, 330, 88, 26));
        lblZ80 = new QLabel(centralwidget);
        lblZ80->setObjectName("lblZ80");
        lblZ80->setGeometry(QRect(1080, 10, 471, 81));
        QFont font;
        font.setFamilies({QString::fromUtf8("Ubuntu Sans Mono")});
        lblZ80->setFont(font);
        lblZ80->setAutoFillBackground(false);
        lblZ80->setFrameShape(QFrame::Box);
        lblZ80->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        lblStack = new QLabel(centralwidget);
        lblStack->setObjectName("lblStack");
        lblStack->setGeometry(QRect(1440, 100, 111, 221));
        lblStack->setFont(font);
        lblStack->setAutoFillBackground(false);
        lblStack->setFrameShape(QFrame::Box);
        lblStack->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        lblDisassembler = new QLabel(centralwidget);
        lblDisassembler->setObjectName("lblDisassembler");
        lblDisassembler->setGeometry(QRect(1080, 100, 351, 221));
        lblDisassembler->setFont(font);
        lblDisassembler->setAutoFillBackground(false);
        lblDisassembler->setFrameShape(QFrame::Box);
        lblDisassembler->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        btnStepOut = new QPushButton(centralwidget);
        btnStepOut->setObjectName("btnStepOut");
        btnStepOut->setGeometry(QRect(1370, 330, 88, 26));
        btnStepOver = new QPushButton(centralwidget);
        btnStepOver->setObjectName("btnStepOver");
        btnStepOver->setGeometry(QRect(1280, 330, 88, 26));
        lblCRTC = new QLabel(centralwidget);
        lblCRTC->setObjectName("lblCRTC");
        lblCRTC->setGeometry(QRect(1080, 370, 471, 61));
        lblCRTC->setFont(font);
        lblCRTC->setAutoFillBackground(false);
        lblCRTC->setFrameShape(QFrame::Box);
        lblCRTC->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        lblGateArray = new QLabel(centralwidget);
        lblGateArray->setObjectName("lblGateArray");
        lblGateArray->setGeometry(QRect(1080, 440, 471, 81));
        lblGateArray->setFont(font);
        lblGateArray->setAutoFillBackground(false);
        lblGateArray->setFrameShape(QFrame::Box);
        lblGateArray->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        btnRender = new QPushButton(centralwidget);
        btnRender->setObjectName("btnRender");
        btnRender->setGeometry(QRect(1080, 330, 88, 26));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1565, 23));
        menuHello = new QMenu(menubar);
        menuHello->setObjectName("menuHello");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menuHello->menuAction());

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Abalore Amstrad Emulator", nullptr));
        btnStepIn->setText(QCoreApplication::translate("MainWindow", "Step in", nullptr));
        lblZ80->setText(QCoreApplication::translate("MainWindow", "Z80", nullptr));
        lblStack->setText(QCoreApplication::translate("MainWindow", "Stack", nullptr));
        lblDisassembler->setText(QCoreApplication::translate("MainWindow", "Disassembler", nullptr));
        btnStepOut->setText(QCoreApplication::translate("MainWindow", "Step out", nullptr));
        btnStepOver->setText(QCoreApplication::translate("MainWindow", "Step over", nullptr));
        lblCRTC->setText(QCoreApplication::translate("MainWindow", "<html><head/><body><p>CRTC</p></body></html>", nullptr));
        lblGateArray->setText(QCoreApplication::translate("MainWindow", "GateArray", nullptr));
        btnRender->setText(QCoreApplication::translate("MainWindow", "Render", nullptr));
        menuHello->setTitle(QCoreApplication::translate("MainWindow", "Hello", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H

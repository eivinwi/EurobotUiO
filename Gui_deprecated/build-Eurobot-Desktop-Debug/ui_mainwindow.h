/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QComboBox *combo_mode;
    QSlider *slider_speedR;
    QSlider *slider_speedL;
    QCheckBox *check_reg;
    QCheckBox *check_time;
    QSpinBox *spin_accel;
    QLCDNumber *display_version;
    QPushButton *button_stop;
    QLCDNumber *display_error;
    QLabel *label_volt;
    QLabel *label_version;
    QLabel *label_error;
    QLabel *label_accel;
    QLabel *label_setspeedL;
    QLabel *label_setspeedR;
    QLabel *label_currentL;
    QLabel *label_currentR;
    QLabel *label_encoderL;
    QLabel *label_encoderR;
    QPushButton *button_reset;
    QSpinBox *display_voltage;
    QSpinBox *display_currentL;
    QSpinBox *display_currentR;
    QSpinBox *display_encoderL;
    QSpinBox *display_encoderR;
    QSpinBox *display_diffL;
    QSpinBox *display_diffR;
    QSpinBox *display_speedL;
    QSpinBox *display_speedR;
    QSpinBox *display_accel;
    QSlider *slider_both;
    QTextBrowser *display_text;
    QPushButton *button_refresh;
    QSpinBox *display_mode;
    QPlainTextEdit *display_textEncL;
    QTextEdit *display_textEncR;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(665, 476);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        combo_mode = new QComboBox(centralWidget);
        combo_mode->setObjectName(QStringLiteral("combo_mode"));
        combo_mode->setGeometry(QRect(10, 90, 85, 27));
        slider_speedR = new QSlider(centralWidget);
        slider_speedR->setObjectName(QStringLiteral("slider_speedR"));
        slider_speedR->setGeometry(QRect(80, 360, 160, 16));
        slider_speedR->setMaximum(255);
        slider_speedR->setValue(128);
        slider_speedR->setOrientation(Qt::Horizontal);
        slider_speedL = new QSlider(centralWidget);
        slider_speedL->setObjectName(QStringLiteral("slider_speedL"));
        slider_speedL->setGeometry(QRect(80, 330, 160, 16));
        slider_speedL->setMaximum(255);
        slider_speedL->setValue(128);
        slider_speedL->setOrientation(Qt::Horizontal);
        check_reg = new QCheckBox(centralWidget);
        check_reg->setObjectName(QStringLiteral("check_reg"));
        check_reg->setGeometry(QRect(10, 40, 97, 22));
        check_reg->setChecked(true);
        check_time = new QCheckBox(centralWidget);
        check_time->setObjectName(QStringLiteral("check_time"));
        check_time->setGeometry(QRect(10, 60, 97, 22));
        spin_accel = new QSpinBox(centralWidget);
        spin_accel->setObjectName(QStringLiteral("spin_accel"));
        spin_accel->setGeometry(QRect(110, 140, 51, 27));
        spin_accel->setMaximum(10);
        spin_accel->setValue(5);
        display_version = new QLCDNumber(centralWidget);
        display_version->setObjectName(QStringLiteral("display_version"));
        display_version->setGeometry(QRect(80, 10, 64, 23));
        display_version->setSegmentStyle(QLCDNumber::Flat);
        button_stop = new QPushButton(centralWidget);
        button_stop->setObjectName(QStringLiteral("button_stop"));
        button_stop->setGeometry(QRect(560, 340, 97, 71));
        display_error = new QLCDNumber(centralWidget);
        display_error->setObjectName(QStringLiteral("display_error"));
        display_error->setGeometry(QRect(590, 300, 64, 23));
        display_error->setSegmentStyle(QLCDNumber::Flat);
        label_volt = new QLabel(centralWidget);
        label_volt->setObjectName(QStringLiteral("label_volt"));
        label_volt->setGeometry(QRect(350, 310, 67, 17));
        label_version = new QLabel(centralWidget);
        label_version->setObjectName(QStringLiteral("label_version"));
        label_version->setGeometry(QRect(10, 10, 67, 17));
        label_error = new QLabel(centralWidget);
        label_error->setObjectName(QStringLiteral("label_error"));
        label_error->setGeometry(QRect(520, 300, 67, 17));
        label_accel = new QLabel(centralWidget);
        label_accel->setObjectName(QStringLiteral("label_accel"));
        label_accel->setGeometry(QRect(10, 140, 101, 17));
        label_setspeedL = new QLabel(centralWidget);
        label_setspeedL->setObjectName(QStringLiteral("label_setspeedL"));
        label_setspeedL->setGeometry(QRect(0, 330, 67, 17));
        label_setspeedR = new QLabel(centralWidget);
        label_setspeedR->setObjectName(QStringLiteral("label_setspeedR"));
        label_setspeedR->setGeometry(QRect(0, 360, 67, 17));
        label_currentL = new QLabel(centralWidget);
        label_currentL->setObjectName(QStringLiteral("label_currentL"));
        label_currentL->setGeometry(QRect(350, 340, 67, 17));
        label_currentR = new QLabel(centralWidget);
        label_currentR->setObjectName(QStringLiteral("label_currentR"));
        label_currentR->setGeometry(QRect(350, 370, 67, 17));
        label_encoderL = new QLabel(centralWidget);
        label_encoderL->setObjectName(QStringLiteral("label_encoderL"));
        label_encoderL->setGeometry(QRect(70, 245, 67, 17));
        label_encoderR = new QLabel(centralWidget);
        label_encoderR->setObjectName(QStringLiteral("label_encoderR"));
        label_encoderR->setGeometry(QRect(70, 275, 67, 17));
        button_reset = new QPushButton(centralWidget);
        button_reset->setObjectName(QStringLiteral("button_reset"));
        button_reset->setGeometry(QRect(10, 240, 51, 61));
        display_voltage = new QSpinBox(centralWidget);
        display_voltage->setObjectName(QStringLiteral("display_voltage"));
        display_voltage->setGeometry(QRect(420, 310, 48, 27));
        display_voltage->setFrame(true);
        display_voltage->setReadOnly(true);
        display_voltage->setButtonSymbols(QAbstractSpinBox::NoButtons);
        display_currentL = new QSpinBox(centralWidget);
        display_currentL->setObjectName(QStringLiteral("display_currentL"));
        display_currentL->setGeometry(QRect(420, 340, 48, 27));
        display_currentL->setFrame(true);
        display_currentL->setReadOnly(true);
        display_currentL->setButtonSymbols(QAbstractSpinBox::NoButtons);
        display_currentR = new QSpinBox(centralWidget);
        display_currentR->setObjectName(QStringLiteral("display_currentR"));
        display_currentR->setGeometry(QRect(420, 370, 48, 27));
        display_currentR->setFrame(true);
        display_currentR->setReadOnly(true);
        display_currentR->setButtonSymbols(QAbstractSpinBox::NoButtons);
        display_encoderL = new QSpinBox(centralWidget);
        display_encoderL->setObjectName(QStringLiteral("display_encoderL"));
        display_encoderL->setGeometry(QRect(230, 70, 81, 27));
        display_encoderL->setFrame(true);
        display_encoderL->setReadOnly(true);
        display_encoderL->setButtonSymbols(QAbstractSpinBox::NoButtons);
        display_encoderR = new QSpinBox(centralWidget);
        display_encoderR->setObjectName(QStringLiteral("display_encoderR"));
        display_encoderR->setGeometry(QRect(230, 100, 81, 27));
        display_encoderR->setFrame(true);
        display_encoderR->setReadOnly(true);
        display_encoderR->setButtonSymbols(QAbstractSpinBox::NoButtons);
        display_diffL = new QSpinBox(centralWidget);
        display_diffL->setObjectName(QStringLiteral("display_diffL"));
        display_diffL->setGeometry(QRect(240, 240, 61, 27));
        display_diffL->setFrame(true);
        display_diffL->setReadOnly(true);
        display_diffL->setButtonSymbols(QAbstractSpinBox::NoButtons);
        display_diffR = new QSpinBox(centralWidget);
        display_diffR->setObjectName(QStringLiteral("display_diffR"));
        display_diffR->setGeometry(QRect(240, 270, 61, 27));
        display_diffR->setFrame(true);
        display_diffR->setReadOnly(true);
        display_diffR->setButtonSymbols(QAbstractSpinBox::NoButtons);
        display_speedL = new QSpinBox(centralWidget);
        display_speedL->setObjectName(QStringLiteral("display_speedL"));
        display_speedL->setGeometry(QRect(250, 320, 48, 27));
        display_speedL->setFrame(true);
        display_speedL->setReadOnly(true);
        display_speedL->setButtonSymbols(QAbstractSpinBox::NoButtons);
        display_speedL->setMaximum(255);
        display_speedR = new QSpinBox(centralWidget);
        display_speedR->setObjectName(QStringLiteral("display_speedR"));
        display_speedR->setGeometry(QRect(250, 350, 48, 27));
        display_speedR->setFrame(true);
        display_speedR->setReadOnly(true);
        display_speedR->setButtonSymbols(QAbstractSpinBox::NoButtons);
        display_speedR->setMaximum(255);
        display_accel = new QSpinBox(centralWidget);
        display_accel->setObjectName(QStringLiteral("display_accel"));
        display_accel->setGeometry(QRect(160, 140, 21, 27));
        display_accel->setFrame(true);
        display_accel->setReadOnly(true);
        display_accel->setButtonSymbols(QAbstractSpinBox::NoButtons);
        display_accel->setMaximum(10);
        slider_both = new QSlider(centralWidget);
        slider_both->setObjectName(QStringLiteral("slider_both"));
        slider_both->setGeometry(QRect(80, 390, 160, 16));
        slider_both->setMaximum(255);
        slider_both->setValue(128);
        slider_both->setOrientation(Qt::Horizontal);
        display_text = new QTextBrowser(centralWidget);
        display_text->setObjectName(QStringLiteral("display_text"));
        display_text->setGeometry(QRect(410, 0, 256, 192));
        button_refresh = new QPushButton(centralWidget);
        button_refresh->setObjectName(QStringLiteral("button_refresh"));
        button_refresh->setGeometry(QRect(380, 280, 97, 26));
        display_mode = new QSpinBox(centralWidget);
        display_mode->setObjectName(QStringLiteral("display_mode"));
        display_mode->setGeometry(QRect(110, 90, 48, 27));
        display_mode->setReadOnly(true);
        display_mode->setButtonSymbols(QAbstractSpinBox::NoButtons);
        display_mode->setMaximum(255);
        display_textEncL = new QPlainTextEdit(centralWidget);
        display_textEncL->setObjectName(QStringLiteral("display_textEncL"));
        display_textEncL->setGeometry(QRect(150, 240, 91, 31));
        display_textEncL->setReadOnly(true);
        display_textEncR = new QTextEdit(centralWidget);
        display_textEncR->setObjectName(QStringLiteral("display_textEncR"));
        display_textEncR->setGeometry(QRect(150, 280, 91, 31));
        display_textEncR->setReadOnly(true);
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 665, 25));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
        combo_mode->clear();
        combo_mode->insertItems(0, QStringList()
         << QApplication::translate("MainWindow", "Mode 0", 0)
         << QApplication::translate("MainWindow", "Mode 1", 0)
         << QApplication::translate("MainWindow", "Mode 2", 0)
        );
        check_reg->setText(QApplication::translate("MainWindow", "Regulator", 0));
        check_time->setText(QApplication::translate("MainWindow", "Timeout", 0));
        button_stop->setText(QApplication::translate("MainWindow", "STOP", 0));
        label_volt->setText(QApplication::translate("MainWindow", "Voltage", 0));
        label_version->setText(QApplication::translate("MainWindow", "Version", 0));
        label_error->setText(QApplication::translate("MainWindow", "Error", 0));
        label_accel->setText(QApplication::translate("MainWindow", "Acceleration", 0));
        label_setspeedL->setText(QApplication::translate("MainWindow", "Speed L", 0));
        label_setspeedR->setText(QApplication::translate("MainWindow", "Speed R", 0));
        label_currentL->setText(QApplication::translate("MainWindow", "Current L", 0));
        label_currentR->setText(QApplication::translate("MainWindow", "Current R", 0));
        label_encoderL->setText(QApplication::translate("MainWindow", "Encoder L", 0));
        label_encoderR->setText(QApplication::translate("MainWindow", "Encoder R", 0));
        button_reset->setText(QApplication::translate("MainWindow", "RESET", 0));
        button_refresh->setText(QApplication::translate("MainWindow", "Refresh", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H

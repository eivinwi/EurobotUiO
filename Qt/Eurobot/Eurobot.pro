#-------------------------------------------------
#
# Project created by QtCreator 2014-10-23T12:53:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Eurobot
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    serial.cpp \
    speedcontrol.cpp

HEADERS  += mainwindow.h \
    serial.h \
    speedcontrol.h

FORMS    += mainwindow.ui

unix|win32: LIBS += -lserial

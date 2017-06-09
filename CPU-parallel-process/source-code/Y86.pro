#-------------------------------------------------
#
# Project created by QtCreator 2016-12-07T19:58:33
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Y86
TEMPLATE = app


SOURCES = main.cpp\
        simulator.cpp \
    global.cpp \
    CPU.cpp

HEADERS  = simulator.h \
    CPU.h \
    global.h

FORMS    = simulator.ui

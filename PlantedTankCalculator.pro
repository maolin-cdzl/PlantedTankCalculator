#-------------------------------------------------
#
# Project created by QtCreator 2019-09-28T01:03:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PlantedTankCalculator
TEMPLATE = app


CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS LIBCONFIG_STATIC LIBCONFIGXX_STATIC



# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    elements.cpp

HEADERS += \
        mainwindow.h \
    elements.h

FORMS += \
        mainwindow.ui



DISTFILES += \
    ptc.cfg \
    ptc_zn_CN.ts

RESOURCES += \
    ptc.qrc

TRANSLATIONS += \
    ptc_zn_CN.ts

win32: INCLUDEPATH += $$PWD/libconfig/include
win32: LIBS += $$PWD/libconfig/lib32/libconfig.a
unix: LIBS += -lconfig++

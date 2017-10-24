#-------------------------------------------------
#
# Project created by QtCreator 2017-09-28T09:47:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SensorGUI_1
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
        $$PWD/FLIR/lib \
        $$PWD/FLIR/include

DEPENDPATH += \
        $$PWD/FLIR/lib \
        $$PWD/FLIR/include

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    logger.cpp \
    flir.cpp

HEADERS += \
        mainwindow.h \
    logger.h \
    flir.h

FORMS += \
        mainwindow.ui

unix: LIBS += -L$$PWD/FLIR/lib/ -lSimpleImagingLib \
-L$$PWD/FLIR/lib/ -lPvVirtualDevice \
-L$$PWD/FLIR/lib/ -lPvTransmitter \
-L$$PWD/FLIR/lib/ -lPvStream \
-L$$PWD/FLIR/lib/ -lPvSerial \
-L$$PWD/FLIR/lib/ -lPvPersistence \
-L$$PWD/FLIR/lib/ -lPvGUI \
-L$$PWD/FLIR/lib/ -lPvGenICam \
-L$$PWD/FLIR/lib/ -lPvDevice \
-L$$PWD/FLIR/lib/ -lPvBuffer \
-L$$PWD/FLIR/lib/ -lPvBase \
-L$$PWD/FLIR/lib/ -lPvAppUtils \
-L$$PWD/FLIR/lib/ -lPtUtilsLib \
-L$$PWD/FLIR/lib/ -lPtConvertersLib \
-L$$PWD/FLIR/lib/ -lEbUtilsLib \
-L$$PWD/FLIR/lib/ -lEbTransportLayerLib \

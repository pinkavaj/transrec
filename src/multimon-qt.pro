#-------------------------------------------------
#
# Project created by QtCreator 2011-04-25T02:43:52
#
#-------------------------------------------------

LIBS     += -lportaudio -lmultimon -lsndfile
QT       += core gui

TARGET = multimon-qt
TEMPLATE = app

win32 {
        INCLUDEPATH += "c:\Program Files\Mega-Nerd\libsndfile\include"
}

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

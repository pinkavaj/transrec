#-------------------------------------------------
#
# Project created by QtCreator 2011-04-25T02:43:52
#
#-------------------------------------------------

LIBS     += -lmultimon -lportaudio
# UNIX only
#LIBS     +=  -lsndfile
QT       += core gui

TARGET = multimon-qt
TEMPLATE = app

win32 {
        INCLUDEPATH += "C:\Program Files\Mega-Nerd\libsndfile\include"
        INCLUDEPATH += "..\..\multimon-lib\src\include"
        INCLUDEPATH += "C:\portaudio\include"

        LIBS     += -L"C:\Program Files\Mega-Nerd\libsndfile\lib" libsndfile-1.dll
        LIBS     += -L"..\..\multimon-lib\src\build"
        LIBS     += -L"C:\portaudio\lib\.libs" 
}

unix {
    INCLUDEPATH += /usr/local/include
    LIBS     += -L/usr/local/lib -lsndfile
}

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

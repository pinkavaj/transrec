#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "portaudio.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    PaError paError;

    ui->setupUi(this);
    paError = Pa_Initialize();
    if (paError != paNoError)
        QMessageBox::critical(this, "Init err", "bagr");
}

MainWindow::~MainWindow()
{
    PaError paError;

    paError = Pa_Terminate();
    if (paError != paNoError)
        ;;
    delete ui;
}

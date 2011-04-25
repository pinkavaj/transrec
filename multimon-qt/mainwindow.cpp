#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    PaError paError;

    ui->setupUi(this);
    paError = Pa_Initialize();
    if (paError != paNoError) {
        QMessageBox::critical(this, "Pa_Initialize error",
                              QString("error: %1").arg(paError));
        this->close();
        return;
    }

    struct PaStreamParameters inputPar;
    inputPar.channelCount = 1;
    inputPar.device = Pa_GetDefaultInputDevice();
    inputPar.sampleFormat = paInt16;
    inputPar.hostApiSpecificStreamInfo = NULL;
    inputPar.suggestedLatency = 1;

    paError = Pa_OpenStream(&stream, &inputPar, NULL, 22050, 0,
                                   paNoFlag, paCallBack_, this);
    if (paError != paNoError) {
        QMessageBox::critical(this, "Pa_OpenDefaultStream error",
                              QString("error: %1").arg(paError));
        this->close();
        return;
    }

    paError = Pa_StartStream(this->stream);
    if (paError != paNoError) {
        QMessageBox::critical(this, "Pa_StartStream error",
                              QString("error: %1").arg(paError));
        this->close();
        return;
    }
}


int MainWindow::paCallBack(const void *input, void *output,
                           unsigned long frameCount,
                           const PaStreamCallbackTimeInfo *timeInfo,
                           PaStreamCallbackFlags statusFlags)
{
    ui->eventsSpinBox->setValue(
                ui->eventsSpinBox->value() + 1);
    return paContinue;
}

int MainWindow::paCallBack_(const void *input, void *output,
                                   unsigned long frameCount,
                                   const PaStreamCallbackTimeInfo *timeInfo,
                                   PaStreamCallbackFlags statusFlags,
                                   void *userData)
{
    MainWindow* mainWin = (MainWindow*)userData;
    return mainWin->paCallBack(input, output, frameCount, timeInfo, statusFlags);
}

MainWindow::~MainWindow()
{
    PaError paError;

    if (this->stream != NULL) {
        paError = Pa_CloseStream(this->stream);
        if (paError != paNoError)
            QMessageBox::critical(this, "Pa_Terminate error",
                                  QString("error: %1").arg(paError));
    }

    paError = Pa_Terminate();
    if (paError != paNoError)
        QMessageBox::critical(this, "Pa_Terminate error",
                              QString("error: %1").arg(paError));
    delete ui;
}

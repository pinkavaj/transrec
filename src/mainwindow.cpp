#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>

const char MainWindow::cfgLogFileName[] = "log file name";
const char MainWindow::cfgRecDirname[] = "rec dir name";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    PaError paError;

    ui->setupUi(this);
    ui->logFileNameLineEdit->setText(
                settings.value(cfgLogFileName, QString()).toString());
    ui->recDirNameLineEdit->setText(
                settings.value(cfgRecDirname, QString()).toString());

    paError = Pa_Initialize();
    if (paError != paNoError) {
        QMessageBox::critical(this, "Pa_Initialize error.",
                              QString("Error: %1").arg(paError));
        this->close();
        return;
    }

    struct PaStreamParameters inputPar;
    inputPar.channelCount = 1;
    inputPar.device = Pa_GetDefaultInputDevice();
    inputPar.sampleFormat = paFloat32;
    inputPar.hostApiSpecificStreamInfo = NULL;
    inputPar.suggestedLatency = 1;

    paError = Pa_OpenStream(&stream, &inputPar, NULL, demod_zvei.samplerate,
                            0, paNoFlag, paCallBack_, this);
    if (paError != paNoError) {
        QMessageBox::critical(this, "Pa_OpenStream error.",
                              QString("Error: %1").arg(paError));
        this->close();
        return;
    }

    demod_zvei.init(&zvei_st);
    // TODO: ...
    paError = Pa_StartStream(this->stream);
    if (paError != paNoError) {
        QMessageBox::critical(this, "Pa_StartStream error.",
                              QString("Error: %1").arg(paError));
        this->close();
        return;
    }
}

MainWindow::~MainWindow()
{
    PaError paError;

    settings.setValue(cfgLogFileName, ui->logFileNameLineEdit->text());
    settings.setValue(cfgRecDirname, ui->recDirNameLineEdit->text());

    if (this->stream != NULL) {
        paError = Pa_CloseStream(this->stream);
        if (paError != paNoError)
            QMessageBox::critical(this, "Pa_CloseStream error.",
                                  QString("Error: %1").arg(paError));
    }

    paError = Pa_Terminate();
    if (paError != paNoError)
        QMessageBox::critical(this, "Pa_Terminate error",
                              QString("Error: %1").arg(paError));
    delete ui;
}

void MainWindow::on_logCheckBox_toggled(bool checked)
{
    if (checked) {
        logFile.setFileName(ui->logFileNameLineEdit->text());

        if (!logFile.open(QFile::Append)) {
            ui->logCheckBox->setChecked(false);

            QString msgTitle("Failed to create/open log file.");
            QString msgText("Failed to create/open file: \"%1\"\n"
                            "with error: %2.");

            msgText = msgText.arg(logFile.fileName(), logFile.errorString());
            QMessageBox::critical(this, msgTitle, msgText);
            return;
        }
    } else
        logFile.close();

    ui->logFileToolButton->setEnabled(!checked);
    ui->logFileNameLineEdit->setReadOnly(checked);
}

void MainWindow::on_logFileToolButton_clicked()
{
    QString logFileName;

    logFileName = QFileDialog::getSaveFileName(this, "Get log file name.");
    ui->logFileNameLineEdit->setText(logFileName);
}

void MainWindow::on_recCheckBox_toggled(bool checked)
{    
    if (checked) {
        QDir recDir;
        QString recDirName;

        recDirName = ui->recDirNameLineEdit->text();
        if (recDirName.isEmpty()) {
            ui->recCheckBox->setChecked(false);
            return;
        }

        recDir.setPath(recDirName);
        if (!recDir.exists()) {
            QMessageBox::StandardButton createRecDir;
            QString msgTitle("Create directory for recording?");
            QString msgText("Selected directory does not exists.\n"
                            "Create directory for recoring?\n"
                            "\"%1\"");

            msgText = msgText.arg(recDirName);
            createRecDir = QMessageBox::question(
                        this, msgTitle, msgText,
                        QMessageBox::Yes | QMessageBox::No);
            if (createRecDir != QMessageBox::Yes) {
                ui->recCheckBox->setChecked(false);
                return;
            }
            if (!recDir.mkpath(recDir.absolutePath())) {
                ui->recCheckBox->setChecked(false);

                QString msgTitle("Failed to create directory.");
                QString msgText("Failed to create directory: \"%1\".");

                msgText = msgText.arg(recDir.path());
                QMessageBox::critical(this, msgTitle, msgText);
                return;
            }
        }
    }

    ui->recDirNameLineEdit->setReadOnly(checked);
    ui->recDirNameToolButton->setEnabled(!checked);
}

void MainWindow::on_recDirNameToolButton_clicked()
{
    QString recDirName;

    recDirName = QFileDialog::getExistingDirectory(
                this, "Get recording directory name.");
    ui->recDirNameLineEdit->setText(recDirName);
}

int MainWindow::paCallBack(const void *input, void *output,
                           unsigned long frameCount,
                           const PaStreamCallbackTimeInfo *timeInfo,
                           PaStreamCallbackFlags statusFlags)
{
    float *buf = (float *)input;

    // TODO: size
    demod_zvei.demod(&zvei_st, buf, frameCount);
    if (ui->logCheckBox->isChecked()) {
        // TODO: write output to file
    }
    return paContinue;
}

int MainWindow::paCallBack_(const void *input, void *output,
                                   unsigned long frameCount,
                                   const PaStreamCallbackTimeInfo *timeInfo,
                                   PaStreamCallbackFlags statusFlags,
                                   void *userData)
{
    MainWindow* mainWin = (MainWindow*)userData;
    return mainWin->paCallBack(input, output, frameCount, timeInfo,
                               statusFlags);
}

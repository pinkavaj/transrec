#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "math.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTime>

const char MainWindow::cfgLogFileName[] = "log file name";
const char MainWindow::cfgRecDirname[] = "rec dir name";
const char MainWindow::cfgRecPwrTreshold[] = "rec power treshold";
const int MainWindow::carrierSampleLen = 20;
const int MainWindow::noCarrierRecLen = 2 * 1000;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    PaError paError;

    recWavFile = NULL;
    carrierLastFrame = 0;
    carrierPwrRemainFrames = 0;
    carrierPwr = 0;
    noCarrierRecDowncount = 0;

    ui->setupUi(this);
    ui->logFileNameLineEdit->setText(
                settings.value(cfgLogFileName, QString()).toString());
    ui->recDirNameLineEdit->setText(
                settings.value(cfgRecDirname, QString()).toString());
    ui->carrierPwrTresholdDoubleSpinBox->setValue(
                settings.value(cfgRecPwrTreshold, 0).toDouble());

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

    demod_zvei.init(&zvei_st, zveiCallback_, this);

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

    settings.setValue(cfgLogFileName, ui->logFileNameLineEdit->text());
    settings.setValue(cfgRecDirname, ui->recDirNameLineEdit->text());
    settings.setValue(cfgRecPwrTreshold, ui->carrierPwrTresholdDoubleSpinBox->value());

    delete recWavFile;
    delete ui;
}

bool MainWindow::hasCarrier(float *buf, int frameCount)
{
    int frameIdx = 0;
    float carrierPwrMax = 0;

    while (frameCount) {
        while (carrierPwrRemainFrames && frameCount) {
            float dl;

            dl = carrierLastFrame - buf[frameIdx];
            carrierPwr += dl * dl;
            carrierLastFrame = buf[frameIdx];

            frameIdx++;
            carrierPwrRemainFrames--;
            frameCount--;
        }

        if (carrierPwrRemainFrames == 0) {
            carrierPwrRemainFrames = demod_zvei.samplerate * carrierSampleLen / 1000;

            if (carrierPwr >= ui->carrierPwrTresholdDoubleSpinBox->value()) {
                carrierLastFrame = buf[frameIdx - 1 + frameCount - 1];
                ui->carrierPwrDoubleSpinBox->setValue(carrierPwr);
                carrierPwr = 0;
                return true;
            }
            if (carrierPwr > carrierPwrMax)
                carrierPwrMax = carrierPwr;

            carrierPwr = 0;
        }
    }
    ui->carrierPwrDoubleSpinBox->setValue(carrierPwrMax);

    return false;
}

bool MainWindow::isRecording()
{
    return recWavFile != NULL;
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
        recStart();
    }
    else
        recStop();

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

    demod_zvei.demod(&zvei_st, buf, frameCount);
    // TODO: get demodulated data from zvei
    if (logFile.isOpen()) {
        // TODO: write output to file
    }

    // TODO: handle bagin/end of transmission (open/close rec file)
    if (hasCarrier(buf, frameCount)) {
        recStart();
        noCarrierRecDowncount = noCarrierRecLen;
    }

    if (isRecording()) {
        if (recWavFile->write(buf, frameCount) != frameCount)
            recStop();

        if (noCarrierRecDowncount <= 0) {
            recStop();
        } else
            noCarrierRecDowncount -= frameCount * 1000 / demod_zvei.samplerate;
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

void MainWindow::recStart()
{
    if (isRecording())
        return;

    QDateTime t = QDateTime::currentDateTime();
    QDateTime tu = t.toUTC();
    QString recDirName = ui->recDirNameLineEdit->text();
    int zoneOffs;

    tu.setTimeSpec(t.timeSpec());
    zoneOffs = tu.secsTo(t) / 3600 * 100;

    if (zoneOffs >= 0)
        recDirName = recDirName + "/rec_%1+%2.wav";
    else
        recDirName = recDirName + "/rec_%1%2.wav";
    recDirName = recDirName.arg(t.toString(Qt::ISODate));
    recDirName = recDirName.arg(zoneOffs, 4, 10, QLatin1Char('0'));
    recFile.setFileName(recDirName);

    if (recFile.open(QFile::WriteOnly)) {
        const int format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
        const int channels = 1;

        recWavFile = new SndfileHandle(
                    recFile.handle(), false, SFM_WRITE, format, channels,
                    demod_zvei.samplerate);
        if (recWavFile->error()) {
            recFile.close();
            delete recWavFile;
            recWavFile = NULL;
        }
    }
}

void MainWindow::recStop()
{
    delete recWavFile;
    recWavFile = NULL;
    recFile.close();
}

void MainWindow::on_carrierPwrTresholdHorizontalSlider_sliderMoved(int position)
{
    double pos;

    position *= position;
    pos = (double)position / 1000;
    ui->carrierPwrTresholdDoubleSpinBox->setValue(pos);
}

void MainWindow::on_carrierPwrTresholdDoubleSpinBox_valueChanged(double value)
{
    int val;

    val = round(sqrt(value * 1000));
    ui->carrierPwrTresholdHorizontalSlider->setValue(val);
}

void MainWindow::zveiCallback_(void *p, int state, const unsigned char *data,
                               int len)
{
    ((MainWindow*)p)->zveiCallback(state, data, len);
}

void MainWindow::zveiCallback(int state, const unsigned char *data, int len)
{
    int rowIdx = ui->logTableWidget->rowCount();
    QString str;

    ui->logTableWidget->insertRow(rowIdx);
    str = QDateTime::currentDateTime().toString(Qt::ISODate);
    ui->logTableWidget->setItem(rowIdx, 0, new QTableWidgetItem(str));
    str = QString("%1").arg((int)data);
    ui->logTableWidget->setItem(rowIdx, 1, new QTableWidgetItem(str));
    str = QString("Not available");
    ui->logTableWidget->setItem(rowIdx, 2, new QTableWidgetItem(str));
}

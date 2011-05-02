#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <multimon.h>
#include <portaudio.h>
#include <QFile>
#include <QMainWindow>
#include <QSettings>
#include <sndfile.hh>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_carrierPwrTresholdDoubleSpinBox_valueChanged(double );
    void on_carrierPwrTresholdHorizontalSlider_sliderMoved(int position);
    void on_logCheckBox_toggled(bool checked);
    void on_logFileToolButton_clicked();
    void on_recCheckBox_toggled(bool checked);
    void on_recDirNameToolButton_clicked();

private:
    static const char cfgLogFileName[];
    static const char cfgRecDirname[];
    static const char cfgRecPwrTreshold[];

    QFile logFile;
    QFile recFile;
    SndfileHandle *recWavFile;
    QSettings settings;
    PaStream *stream;
    Ui::MainWindow *ui;
    demod_state zvei_st;

    /* Compute signal power of received broadcast */
    float carrierLastFrame;
    int carrierPwrRemainFrames;
    float carrierPwr;
    // power frame lenght in ms
    static const int carrierSampleLen;
    // lenght (ms) of record, when input silences
    static const int noCarrierRecLen;
    // ms remaining to stop recording
    int noCarrierRecDowncount;

    int paCallBack(const void *input, void *output,
                   unsigned long frameCount,
                   const PaStreamCallbackTimeInfo *timeInfo,
                   PaStreamCallbackFlags statusFlags);
    static int paCallBack_(const void *input, void *output,
                          unsigned long frameCount,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags, void *userData);

    bool hasCarrier(float *buf, int frameCount);
    bool isRecording();
    void recStart();
    void recStop();

    unsigned char zveiLastChar;
    void zveiCallback(int state, const unsigned char *data, int len);
    static void zveiCallback_(void *p, int state, const unsigned char *data,
                              int len);
};

#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <multimon.h>
#include <portaudio.h>
#include <QFile>
#include <QMainWindow>
#include <QSettings>
#include <QTimer>
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
    void on_recTimer_timeout();

private:
    static const char cfgLogFileName[];
    static const char cfgRecDirname[];
    static const char cfgRecPwrTreshold[];

    QFile logFile;
    QFile recFile;

    /** Used to update Qt widgets by recieved data asynchronously. */
    QTimer recTimer;
    SndfileHandle *recWavFile;
    PaStream *stream;
    QSettings settings;
    Ui::MainWindow *ui;
    demod_state zvei_st;

    /** Value of last audio sample. */
    float carrierLastFrame;
    int carrierPwrRemainFrames;

    /** Last computed value of DC signal power. */
    double carrierPwr;

    /** Used to store temporar value diring carrier RMS computation. */
    double carrierPwr_;

    /** Treshold used for start of recording. */
    double carrierPwrTreshold;

    /** Power measurement frame lenght in ms. */
    static const int carrierSampleLen;

    // lenght (ms) of record, when input is silent
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
    void zveiCallback(char data, int state);
    static void zveiCallback_(char data, int state, void *p);
};

#endif // MAINWINDOW_H

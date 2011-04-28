#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <multimon.h>
#include <portaudio.h>
#include <QMainWindow>
#include <QSettings>

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
    void on_logCheckBox_toggled(bool checked);
    void on_logFileToolButton_clicked();
    void on_recCheckBox_toggled(bool checked);
    void on_recDirNameToolButton_clicked();

private:
    static const char cfgLogFileName[];
    static const char cfgRecDirname[];

    QSettings settings;
    PaStream *stream;
    Ui::MainWindow *ui;
    demod_state zvei_st;

    int paCallBack(const void *input, void *output,
                   unsigned long frameCount,
                   const PaStreamCallbackTimeInfo *timeInfo,
                   PaStreamCallbackFlags statusFlags);
    static int paCallBack_(const void *input, void *output,
                          unsigned long frameCount,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags, void *userData);
};

#endif // MAINWINDOW_H

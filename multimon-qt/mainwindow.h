#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "portaudio.h"
#include "../multimon/multimon.h"

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
    void on_logFileToolButton_clicked();
    void on_logFileCheckBox_toggled(bool checked);
    void on_recDirNameToolButton_clicked();

    void on_recCheckBox_toggled(bool checked);

private:
    Ui::MainWindow *ui;
    PaStream *stream;
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

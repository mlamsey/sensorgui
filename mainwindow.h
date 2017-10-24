#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QThread>
#include "logger.h"
#include "flir.h"

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
    void on_StartRecordingButton_clicked();
    void on_StopRecordingButton_clicked();
    void on_connectDevicesButton_clicked();
    void on_FLIRToggle_clicked(bool checked);
    void on_IntertestToggle_clicked(bool checked);
    void on_ueToggle_clicked(bool checked);
    void on_KeyenceToggle_clicked(bool checked);
    void on_MicrophoneToggle_clicked(bool checked);
    void on_ArcAgentToggle_clicked(bool checked);

    // Signals and Slots
    void getLogMessage(QString message, QString messageType, QString threadID);
    void getFLIRFrame(QPixmap img);
    void getFLIRFrameRate(double frameRate);
    void getFLIRConnection(bool state);
    void getFLIRFrameCount(int count);

private:
    Ui::MainWindow *ui;
    logger* log = new logger;
    flir* FLIR = new flir;

    void record();
    void delay(int millisecondsToWait);
    void connectDevices();

    void updateRecordingSize(double bytes);

    // Connect functions
    bool connectFLIR();
    bool connectIntertest();
    bool connectUE();
    bool connectKeyence();
    bool connectMicrophone();
    bool connectArcAgent();

    // Threading
    void initArcAgentThread();
    void initFLIRThread();
    void initIntertestThread();
    void initKeyenceThread();
    void initMicrophoneThread();
    void initUEThread();

    // State Variables
    bool isRecording = false;
    bool FLIRConnected = false;
    bool IntertestConnected = false;
    bool UEConnected = false;
    bool KeyenceConnected = false;
    bool MicrophoneConnected = false;
    bool ArcAgentConnected = false;
    double recordingSize = 0;
    int FLIRFrameCount = 0;

signals:
    void flirRecordingToggle(bool state);
    void connectFLIRSignal();
};

#endif // MAINWINDOW_H

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStyle>
#include <QMenu>
#include <QFileDialog>
#include <QTimerEvent>
#include <QMessageBox>

#include <QCloseEvent>

#include "tgmath.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_spectrograph(new Spectrograph(this)) // Microphone Output
{
    ui->setupUi(this);
    log->makeLog("Sensor GUI: ","debug");

    // Set display elements to black boxes until they're connected
    ui->FLIRDisplay->setStyleSheet("background: black");
    ui->ArcAgentdisplay->setStyleSheet("background: black");
    ui->IntertestDisplay->setStyleSheet("background: black");
    ui->ueDisplay->setStyleSheet("background: black");
    ui->MicrophoneDisplay->setStyleSheet("background: black");
    ui->KeyenceDisplay->setStyleSheet("background: black");

    // Set display frames to grey until connected
    ui->FLIRFrame->setStyleSheet("background: gray");
    ui->ArcAgentFrame->setStyleSheet("background: gray");
    ui->IntertestFrame->setStyleSheet("background: gray");
    ui->ueFrame->setStyleSheet("background: gray");
    ui->MicrophoneFrame->setStyleSheet("background: gray");
    ui->KeyenceFrame->setStyleSheet("background: gray");

    // Disable UI Elements
    ui->StartRecordingButton->setEnabled(false);
    ui->StopRecordingButton->setEnabled(false);
    ui->FLIRToggle->setEnabled(false);
    ui->IntertestToggle->setEnabled(false);
    ui->ueToggle->setEnabled(false);
    ui->KeyenceToggle->setEnabled(false);
    ui->MicrophoneToggle->setEnabled(false);
    ui->ArcAgentToggle->setEnabled(false);

    // Initialize Threads
    initFLIRThread();

    // Initialize Microphone
    setupFFTDisplay();
    setupFFTConnections();
}

MainWindow::~MainWindow()
{
    isRecording = false;
    delete ui;
}

void MainWindow::record()
{
    log->msg("Recording Started.");
    int startTime = QDateTime::currentMSecsSinceEpoch();

    // Lock toggled devices
    ui->FLIRToggle->setEnabled(false);
    ui->IntertestToggle->setEnabled(false);
    ui->ueToggle->setEnabled(false);
    ui->KeyenceToggle->setEnabled(false);
    ui->MicrophoneToggle->setEnabled(false);
    ui->ArcAgentToggle->setEnabled(false);

    // Check connected devices
    if(ui->FLIRToggle->isChecked())
    {
        ui->recordingFeedback->addItem("Recording FLIR.");
        flirRecordingToggle(true);
    }
    if(ui->IntertestToggle->isChecked())
        ui->recordingFeedback->addItem("Recording Intertest.");
    if(ui->ueToggle->isChecked())
        ui->recordingFeedback->addItem("Recording με.");
    if(ui->KeyenceToggle->isChecked())
        ui->recordingFeedback->addItem("Recording Keyence.");
    if(ui->MicrophoneToggle->isChecked())
    {
        ui->recordingFeedback->addItem("Recording Microphone.");
        setMicrophoneSaving(true);
    }
    if(ui->ArcAgentToggle->isChecked())
        ui->recordingFeedback->addItem("Recording Arc Agent.");

    // Log Recording Time
    int prevTime = startTime;
    int thisTime = startTime;
    int recordingTime = (prevTime - startTime)/1000; // convert ms -> sec
    double dT = 0;
    double refreshRate = 0;

    // Recording Loop
    while(isRecording)
    {
        prevTime = thisTime;

        thisTime = QDateTime::currentMSecsSinceEpoch();
        recordingTime = (thisTime - startTime)/1000; // convert ms -> sec
        dT = (thisTime - prevTime);

        if (dT == 0) // infinite speed prevention for refresh < 1ms
        {
            delay(2);
            dT = 2;
        }
        refreshRate = 1000.0/dT; // ms -> Hz

        ui->recordingTime->setText(QString::number(recordingTime) + " sec");
        ui->recordingRate->setText(QString::number(refreshRate) + " Hz");
        QCoreApplication::processEvents(QEventLoop::AllEvents,10);
    }

    setMicrophoneSaving(false);

    log->msg("Recording Stopped.");
    log->statMessage("Recording Time: " + QString::number(recordingTime) + "sec");
    log->statMessage("FLIR Recording Size: " + QString::number(FLIRFrameCount*flir::flirFrameFileSize) + " bytes.");
    ui->recordingFeedback->addItem("Time: " + QString::number(recordingTime) + "sec");

    // Unlock recording devices
    if(FLIRConnected)
        ui->FLIRToggle->setEnabled(true);
    if(IntertestConnected)
        ui->IntertestToggle->setEnabled(true);
    if(UEConnected)
        ui->ueToggle->setEnabled(true);
    if(KeyenceConnected)
        ui->KeyenceToggle->setEnabled(true);
    if(MicrophoneConnected)
        ui->MicrophoneToggle->setEnabled(true);
    if(ArcAgentConnected)
        ui->ArcAgentToggle->setEnabled(true);
}

void MainWindow::initArcAgentThread()
{
    //
}

void MainWindow::initFLIRThread()
{
    QThread* FLIRThread = new QThread;
    //flir* FLIR = new flir;

    FLIR->moveToThread(FLIRThread);

    // Connect to FLIR
    connect(FLIRThread,SIGNAL(started()),FLIR,SLOT(startFLIRThread()));

    // Cleanup Connections
    connect(FLIR,SIGNAL(finished()),FLIRThread,SLOT(quit()));
    connect(FLIR,SIGNAL(finished()),FLIRThread,SLOT(deleteLater()));
    connect(FLIRThread,SIGNAL(finished()),FLIRThread,SLOT(deleteLater()));

    connect(this,SIGNAL(connectFLIRSignal()),FLIR,SLOT(initAcquisition()));
    connect(FLIR,SIGNAL(connection(bool)),this,SLOT(getFLIRConnection(bool)));

    connect(FLIR,SIGNAL(sendLogMessage(QString,QString,QString)),this,SLOT(getLogMessage(QString,QString,QString)));
    connect(FLIR,SIGNAL(sendFrame(QPixmap)),this,SLOT(getFLIRFrame(QPixmap)));
    connect(FLIR,SIGNAL(sendFrameRate(double)),this,SLOT(getFLIRFrameRate(double)));
    connect(this,SIGNAL(flirRecordingToggle(bool)),FLIR,SLOT(record(bool)));
    connect(FLIR,SIGNAL(sendFrameCount(int)),this,SLOT(getFLIRFrameCount(int)));
    FLIRThread->start();
}

void MainWindow::initIntertestThread()
{
    //
}

void MainWindow::initKeyenceThread()
{
    //
}

void MainWindow::initMicrophoneThread()
{
    QThread* microphoneThread = new QThread;
    microphone->moveToThread(microphoneThread);

    microphoneThread->start();
}

void MainWindow::initUEThread()
{
    //
}

void MainWindow::connectDevices()
{
    ui->connectDevicesButton->setEnabled(false);
    log->debugMessage("Connecting Devices...");

    FLIRConnected = connectFLIR();
    IntertestConnected = connectIntertest();
    UEConnected = connectUE();
    KeyenceConnected = connectKeyence();
    MicrophoneConnected = connectMicrophone();
    ArcAgentConnected = connectArcAgent();

    bool allConnected = false;
    if(FLIRConnected && IntertestConnected
            && UEConnected && KeyenceConnected
            && MicrophoneConnected && ArcAgentConnected)
        allConnected = true;

    bool success = true;
    if(!FLIRConnected && !IntertestConnected
            && !UEConnected && !KeyenceConnected
            && !MicrophoneConnected && !ArcAgentConnected)
        success = false;

    if(success)
    {
        ui->recordingFeedback->addItem("Connection successful.");
        ui->StartRecordingButton->setEnabled(true);
    }
    else
    {
        ui->recordingFeedback->addItem("Connection unsuccessful.");
    }

    if(allConnected)
    {
        ui->recordingFeedback->addItem("All Devices Connected.");
        ui->connectDevicesButton->setText("Connected.");
        log->debugMessage("All devices connected.");
    }
    else
        ui->connectDevicesButton->setEnabled(true);
}

bool MainWindow::connectFLIR()
{
    if(!FLIRConnected)
    {
        connectFLIRSignal();
        ui->FLIRRefreshRate->setText("Connecting");
        ui->FLIRDisplay->setText("Connecting...");
        return true;
    }
    else
        return false;
}

bool MainWindow::connectIntertest() // NOT IMPLEMENTED
{
    ui->IntertestFrame->setStyleSheet("background: black");
    ui->IntertestToggle->setEnabled(false);
    //connect
    return false;
}

bool MainWindow::connectUE() // NOT IMPLEMENTED
{
    ui->ueFrame->setStyleSheet("background: black");
    ui->ueToggle->setEnabled(false);
    //connect
    return false;
}

bool MainWindow::connectKeyence() // NOT IMPLEMENTED
{
    ui->KeyenceFrame->setStyleSheet("background: black");
    ui->KeyenceToggle->setEnabled(false);
    //connect
    return false;
}

bool MainWindow::connectMicrophone()
{
    if(!MicrophoneConnected)
    {
        MicrophoneConnected = true;
        ui->MicrophoneFrame->setStyleSheet("background: red");
        ui->MicrophoneToggle->setEnabled(true);
        return true;
    }
    else
    {
        ui->MicrophoneFrame->setStyleSheet("background: black");
        return false;
    }
}

bool MainWindow::connectArcAgent() // NOT IMPLEMENTED
{
    ui->ArcAgentFrame->setStyleSheet("background: black");
    ui->ArcAgentToggle->setEnabled(false);
    //connect
    return false;
}

void MainWindow::setupFFTDisplay()
{
    // Parameters defined in ../spectrum/app/spectrum.h
    m_spectrograph->setParams(SpectrumNumBands, SpectrumLowFreq, SpectrumHighFreq);

    // Place spectrograph in a box
    ui->spectrograph->addWidget(m_spectrograph);

    // Configure spectrograph
    m_spectrograph->setFixedWidth(320);

    // Configure Clip Event List Widget
    QFont clipFont;
    clipFont.setPointSize(10);
    ui->list_clipEvents->setFont(clipFont);

    // Configure Axis Labels
    ui->xAxisLabel->setFixedHeight(20);
    ui->xAxisLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    int xAxisDivisions = 8;
    QString xLabel = QString::number(SpectrumLowFreq);
    int i;

    double increment = 100*(ceil((SpectrumHighFreq - SpectrumLowFreq)/(xAxisDivisions*100)));

    for(i = 1; i < xAxisDivisions - 1; i++)
    {
        xLabel = xLabel + "    " + QString::number(increment*i);
    }

    xLabel = xLabel + "    " + QString::number(SpectrumHighFreq);

    QFont xAxisFont;
    xAxisFont.setPointSize(10);
    ui->xAxisLabel->setFont(xAxisFont);
    ui->xAxisLabel->setText(xLabel);

    // Configure Slider
    ui->gainSlider->setMinimum(1);
    ui->gainSlider->setMaximum(100);
    ui->gainSlider->setTracking(true);
    ui->gainSlider->setValue(15);

    // Configure Colors
    QString widgetColor = "silver";
    ui->MicrophoneFrame->lower();
    ui->MicrophoneDisplay->setStyleSheet("background: " + widgetColor);
    ui->xAxisLabel->setStyleSheet("background: " + widgetColor);
    ui->list_clipEvents->setStyleSheet("background: " + widgetColor);
    ui->label_gain->setStyleSheet("background: " + widgetColor);
    ui->gainSlider->setStyleSheet("background: " + widgetColor);
    ui->label_refreshRate->setStyleSheet("background: " + widgetColor);
}

void MainWindow::setupFFTConnections()
{
    // UI Connections
    connect(ui->connectDevicesButton,SIGNAL(clicked()),microphone,SLOT(runFFT())); // Start button fcn
    connect(microphone,SIGNAL(startEngineRecording()),this,SLOT(updateToggleButton())); // start btn label
    connect(microphone,SIGNAL(pauseEngineRecording()),this,SLOT(updateToggleButton())); // start btn label

    // Engine
    connect(microphone,SIGNAL(sendSpectrum(FrequencySpectrum)), // Pass spectrum
            this,SLOT(getFFTSpectrum(FrequencySpectrum)));
    connect(microphone,SIGNAL(sendRefreshRate(long long)), // Pass refresh rate
            this,SLOT(getFFTRefreshRate(long long)));
    connect(this,SIGNAL(sendFFTVisualizerGain(double)),
            microphone,SLOT(getVisualizerGain(double)));
    connect(this,SIGNAL(stopFFT()),microphone,SLOT(stopFFT()));

    // Spectrograph
    connect(this,SIGNAL(fetchTargetFrequencies()), // Ping for frequencies from fftinterface
            microphone,SLOT(targetFreqSlot()));
    connect(microphone,SIGNAL(sendTargetFrequencies(int*,int*)), // Get frequencies from fftinterface
            this,SLOT(getTargetFrequencies(int*,int*)));
    connect(this,SIGNAL(sendFFTTargetFrequencies(int*,int*)), // Send frequencies to the spectrograph
            m_spectrograph,SLOT(setTargetFrequencies(int*,int*)));
    connect(m_spectrograph,SIGNAL(targetFrequencyIsClipped(int)), // Get target value clipped flag
            this,SLOT(getTargetFrequencyClipped(int)));

    // Calls
    fetchTargetFrequencies();
}

void MainWindow::getFFTSpectrum(const FrequencySpectrum &spectrum)
{
    m_spectrograph->spectrumChanged(spectrum);
}

void MainWindow::getFFTRefreshRate(long long refreshRateUs)
{
    double us = refreshRateUs;
    //qDebug()<<us;
    QString rate = QString::number(1000000/us,'f',0);

    ui->label_refreshRate->setText("Refresh Rate: " + rate +" Hz");
    ui->MicrophoneRefreshRate->setText(QString::number(1000000/us,'f',0)+"Hz");
}

void MainWindow::getTargetFrequencies(int* freq, int* max)
{
    for (int i = 1; i < freq[0]; i++) // copy to local scope
    {
        targetFrequencies[i] = freq[i];
    }

    sendFFTTargetFrequencies(freq,max);
}

void MainWindow::getTargetFrequencyClipped(int freqIndex)
{
    int maxMsgCount = 10;
    int msgResetCount = maxMsgCount - 1; //

    // Send to event list in UI
    QString msg = "Bar " + QString::number(freqIndex) + " at " +
            QString::number(targetFrequencies[freqIndex]) + "Hz";
    ui->list_clipEvents->addItem(msg);

    // Clean up event list to prevent overflow
    if(ui->list_clipEvents->count() > maxMsgCount)
    {
        QString previous[msgResetCount]; // holds old messages
        for (int i = 0; i < msgResetCount; i++)
        {
            previous[i] = ui->list_clipEvents->item(i + 1)->text();
        }

        ui->list_clipEvents->clear(); // clear it out
        for (int i = 0; i < msgResetCount; i++) // ... and repopulate it
        {
            ui->list_clipEvents->addItem(previous[i]);
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event) // overridden
{
    stopFFT();
    event->accept();
    QApplication::quit();
}

void MainWindow::updateToggleButton()
{/*
    if(microphone->running)
        ui->btn_runToggle->setText("Pause");
    else
        ui->btn_runToggle->setText("Start");*/
}

// ----- SLOTS ----- //
void MainWindow::getLogMessage(QString message, QString messageType, QString threadID)
{ // Fetches messages from threads
    // Add thread name to message
    message.insert(0,"(" + threadID + ") ");

    std::string type = messageType.toStdString();
    // Determine message type + write message
    if(type == "SYSTEM")
        log->systemMessage(message);
    else if(type == "ERROR")
        log->errorMessage(message);
    else if(type == "DEBUG")
        log->debugMessage(message);
    else if(type == "STAT")
        log->statMessage(message);
    else
        log->msg("(unknown message type) " + message); // etc
}

void MainWindow::getFLIRFrame(QPixmap img)
{
    ui->FLIRDisplay->setPixmap(img);
    ui->FLIRDisplay->show();
}

void MainWindow::getFLIRFrameRate(double frameRate)
{
    ui->FLIRRefreshRate->setText(QString::number(frameRate,'f',1) + "Hz");
}

void MainWindow::getFLIRConnection(bool state)
{
    FLIRConnected = state;
    if(FLIRConnected)
    {
        ui->FLIRFrame->setStyleSheet("background: red");
        ui->FLIRToggle->setEnabled(true);
        ui->recordingFeedback->addItem("FLIR Connected.");
        log->systemMessage("FLIR Connected.");
    }
    else
    {
        ui->FLIRFrame->setStyleSheet("background: black");
        ui->recordingFeedback->addItem("FLIR Failed to Connect.");
        log->errorMessage("FLIR Failed to Connect.");
        ui->FLIRDisplay->setText("Disconnected.");
        ui->FLIRRefreshRate->setText("n/a");
    }
}

void MainWindow::getFLIRFrameCount(int count)
{
    FLIRFrameCount = count;
    updateRecordingSize(flir::flirFrameFileSize);
}

// ----- Internal Functions ----- //
void MainWindow::updateRecordingSize(double bytes)
{
    double MB = bytes/1048576;
    recordingSize = recordingSize + MB;
    ui->recordingSize->setText(QString::number(recordingSize,'f',2) + "MB");
}

void MainWindow::delay(int millisecondsToWait) // from internet
{
    QTime dieTime = QTime::currentTime().addMSecs( millisecondsToWait );
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
    }
}

// ----- UI CALLBACKS ----- //
void MainWindow::on_StartRecordingButton_clicked()
{
    ui->StartRecordingButton->setEnabled(false);
    ui->StopRecordingButton->setEnabled(true);
    ui->StartRecordingButton->setText("Recording");
    log->UIAction("Start Recording Button Clicked.");
    ui->recordingFeedback->addItem("Recording Started.");
    isRecording = true;
    record();
}

void MainWindow::on_StopRecordingButton_clicked()
{
    ui->StartRecordingButton->setEnabled(true);
    ui->StopRecordingButton->setEnabled(false);
    ui->StartRecordingButton->setText("Start Recording");
    log->UIAction("Stop Recording Button Clicked.");
    ui->recordingFeedback->addItem("Recording Stopped.");
    isRecording = false;
    flirRecordingToggle(false);
}

void MainWindow::on_connectDevicesButton_clicked()
{
    log->UIAction("Connect Devices Button Clicked.");
    connectDevices();
}

void MainWindow::on_FLIRToggle_clicked(bool checked)
{
    if(checked)
        ui->FLIRFrame->setStyleSheet("background: green");
    else
        ui->FLIRFrame->setStyleSheet("background: red");
}

void MainWindow::on_IntertestToggle_clicked(bool checked)
{
    if(checked)
        ui->IntertestFrame->setStyleSheet("background: green");
    else
        ui->IntertestFrame->setStyleSheet("background: red");
}

void MainWindow::on_ueToggle_clicked(bool checked)
{
    if(checked)
        ui->ueFrame->setStyleSheet("background: green");
    else
        ui->ueFrame->setStyleSheet("background: red");
}

void MainWindow::on_KeyenceToggle_clicked(bool checked)
{
    if(checked)
        ui->KeyenceFrame->setStyleSheet("background: green");
    else
        ui->KeyenceFrame->setStyleSheet("background: red");
}

void MainWindow::on_MicrophoneToggle_clicked(bool checked)
{
    if(checked)
        ui->MicrophoneFrame->setStyleSheet("background: green");
    else
        ui->MicrophoneFrame->setStyleSheet("background: red");
}

void MainWindow::on_ArcAgentToggle_clicked(bool checked)
{
    if(checked)
        ui->ArcAgentFrame->setStyleSheet("background: green");
    else
        ui->ArcAgentFrame->setStyleSheet("background: red");
}

void MainWindow::on_gainSlider_valueChanged(int value)
{
    double gain = value; // 1-100
    ui->label_gain->setText("Gain: " + QString::number(value) + "%");

    sendFFTVisualizerGain(gain);
}

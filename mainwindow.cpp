#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
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
        ui->recordingFeedback->addItem("Recording Microphone.");
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

    log->msg("Recording Stopped.");
    log->statMessage("Recording Time: " + QString::number(recordingTime) + "sec");
    log->statMessage("FLIR Recording Size: " + QString::number(FLIRFrameCount*flir::flirFrameFileSize) + " bytes.");
    ui->recordingFeedback->addItem("Time: " + QString::number(recordingTime) + "sec");

    // Unlock recording devices
    ui->FLIRToggle->setEnabled(true);
    ui->IntertestToggle->setEnabled(true);
    ui->ueToggle->setEnabled(true);
    ui->KeyenceToggle->setEnabled(true);
    ui->MicrophoneToggle->setEnabled(true);
    ui->ArcAgentToggle->setEnabled(true);
}

void MainWindow::initArcAgentThread()
{
    //
}

void MainWindow::initFLIRThread()
{
    QThread* FLIRThread = new QThread;
    flir* FLIRWorker = new flir;

    FLIRWorker->moveToThread(FLIRThread);

    // Connect to FLIR
    connect(FLIRThread,SIGNAL(started()),FLIRWorker,SLOT(startFLIRThread()));

    // Cleanup Connections
    connect(FLIRWorker,SIGNAL(finished()),FLIRThread,SLOT(quit()));
    connect(FLIRWorker,SIGNAL(finished()),FLIRThread,SLOT(deleteLater()));
    connect(FLIRThread,SIGNAL(finished()),FLIRThread,SLOT(deleteLater()));

    connect(this,SIGNAL(connectFLIRSignal()),FLIRWorker,SLOT(initAcquisition()));
    connect(FLIRWorker,SIGNAL(connection(bool)),this,SLOT(getFLIRConnection(bool)));

    connect(FLIRWorker,SIGNAL(sendLogMessage(QString,QString,QString)),this,SLOT(getLogMessage(QString,QString,QString)));
    connect(FLIRWorker,SIGNAL(sendFrame(QPixmap)),this,SLOT(getFLIRFrame(QPixmap)));
    connect(FLIRWorker,SIGNAL(sendFrameRate(double)),this,SLOT(getFLIRFrameRate(double)));
    connect(this,SIGNAL(flirRecordingToggle(bool)),FLIRWorker,SLOT(record(bool)));
    connect(FLIRWorker,SIGNAL(sendFrameCount(int)),this,SLOT(getFLIRFrameCount(int)));
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
    //
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

bool MainWindow::connectIntertest()
{
    ui->IntertestFrame->setStyleSheet("background: red");
    ui->IntertestToggle->setEnabled(true);
    //connect
    return true;
}

bool MainWindow::connectUE()
{
    ui->ueFrame->setStyleSheet("background: red");
    ui->ueToggle->setEnabled(true);
    //connect
    return true;
}

bool MainWindow::connectKeyence()
{
    ui->KeyenceFrame->setStyleSheet("background: red");
    ui->KeyenceToggle->setEnabled(true);
    //connect
    return true;
}

bool MainWindow::connectMicrophone()
{
    ui->MicrophoneFrame->setStyleSheet("background: red");
    ui->MicrophoneToggle->setEnabled(true);
    //connect
    return true;
}

bool MainWindow::connectArcAgent()
{
    ui->ArcAgentFrame->setStyleSheet("background: red");
    ui->ArcAgentToggle->setEnabled(true);
    //connect
    return true;
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

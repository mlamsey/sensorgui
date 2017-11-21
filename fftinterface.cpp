#include "fftinterface.h"

#include "engine.h"
#include "waveform.h"
#include "progressbar.h"
#include "settingsdialog.h"
#include "spectrograph.h"
#include "tonegeneratordialog.h"
#include "utils.h"

fftinterface::fftinterface()
    : QObject(),
    m_engine(new Engine(this))
{
    bufferTime = m_engine->BufferDurationUs/1000; // us -> ms
    setupConnections();

    sendTargetFrequencies(targetFrequencies,maximumIntensity);
}

fftinterface::~fftinterface()
{
    running = false;
    m_engine->suspend();
}

void fftinterface::runFFT()
{
    if(!running)
    {
        running = true;
        int currentTime = QDateTime::currentMSecsSinceEpoch();
        int prevTime = currentTime;
        int dT;

        // Begin Recording
        startEngineRecording();

        // Restarts the "recording" after the buffer (engine.h) expires
        while (running)
        {
            currentTime = QDateTime::currentMSecsSinceEpoch();
            dT = currentTime - prevTime;

            if (dT >= bufferTime)
            {
                qDebug()<<"Retriggered FFT Buffer";
                prevTime = currentTime;
                //startEngineRecording();
                resetBuffer();
                startEngineRecording();
            }

            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }//while

    }//if

    else
    {
        running = false;
        pauseEngineRecording();
    }
}

void fftinterface::stopFFT()
{
    running = false;
    stopEngineRecording();
}

void fftinterface::spectrumChanged(qint64 spectrumPosition,
                                   qint64 bufferLength,
                                   const FrequencySpectrum &spectrum)
{
    //qDebug()<<"Spectrum Changed.";
    emit sendSpectrum(spectrum); // routes signal from spectrum to widget

    // Refresh Rate
    prevSpectrumTime = currentSpectrumTime;
    currentSpectrumTime = QDateTime::currentMSecsSinceEpoch();
    int dT = currentSpectrumTime-prevSpectrumTime;
    sendRefreshRate(dT);
}

void fftinterface::startRecording()
{
    startEngineRecording();
    sendRefreshRate(m_engine->NotifyIntervalMs);
    currentSpectrumTime = QDateTime::currentMSecsSinceEpoch();
}

void fftinterface::setupConnections()
{
    // With Engine
    connect(this,SIGNAL(startEngineRecording()),m_engine,SLOT(startRecording()));
    connect(m_engine,SIGNAL(spectrumChanged(qint64,qint64,FrequencySpectrum)),this,SLOT(spectrumChanged(qint64,qint64,FrequencySpectrum)));
    connect(this,SIGNAL(pauseEngineRecording()),m_engine,SLOT(suspend()));
    connect(this,SIGNAL(sendVisualizerGain(double)),m_engine,SLOT(getVisualizerGain(double)));
    connect(this,SIGNAL(resetBuffer()),m_engine,SLOT(resetEngineTimeout()));
    connect(this,SIGNAL(stopEngineRecording()),m_engine,SLOT(stopRecordingSlot()));
}

void fftinterface::getVisualizerGain(double gain)
{
    sendVisualizerGain(gain);
}

void fftinterface::targetFreqSlot()
{
    int* freqptr;
    freqptr = targetFrequencies;
    sendTargetFrequencies(freqptr,maximumIntensity);
}

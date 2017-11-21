#ifndef FFTINTERFACE_H
#define FFTINTERFACE_H

#include "engine.h"
#include "settingsdialog.h"
#include "tonegeneratordialog.h"
#include "utils.h"

#include <QObject>
#include <QDateTime>
#include <QCoreApplication>

class fftinterface : public QObject
{ Q_OBJECT
public:
    fftinterface();
    ~fftinterface();

    // State
    bool running = false;

private:
    // Refresh Rate
    int prevSpectrumTime;
    int currentSpectrumTime;

    // FFT Information
    qint64 bufferTime;

    Engine* m_engine;

    void getFFTStats();
    void setupConnections();
    void startRecording();

    // First element in each of the following arrays must be assigned as the length of the array!
    int targetFrequencies[6] = {6,440,750,1200,2400,3200}; // Hz
    int maximumIntensity[6] = {6,50,38,25,13,7}; // 0-100 scale, relative

private slots:
    // From UI
    void runFFT();
    void stopFFT();

    // From Engine
    void spectrumChanged(qint64 spectrumPosition, qint64 bufferLength, const FrequencySpectrum &spectrum);

public slots:
    void getVisualizerGain(double gain);
    void targetFreqSlot();

signals:
    // To UI
    void sendSpectrum(const FrequencySpectrum &spectrum);
    void sendRefreshRate(int refreshRateMs);

    // To Engine
    void startEngineRecording();
    void pauseEngineRecording();
    void stopEngineRecording(); // dumps data to file and resets buffer
    void sendVisualizerGain(double); // pass to engine
    void resetBuffer();

    // To UI & Engine
    void sendTargetFrequencies(int* frequencies, int* intensityThresholds);
};

#endif // FFTINTERFACE_H

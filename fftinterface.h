#ifndef FFTINTERFACE_H
#define FFTINTERFACE_H

#include "engine.h"
#include "settingsdialog.h"
#include "tonegeneratordialog.h"
#include "utils.h"

#include <QThread>
#include <QObject>
#include <QDateTime>
#include <QCoreApplication>

#include <chrono>

class fftinterface : public QObject
{ Q_OBJECT
public:
    fftinterface();
    ~fftinterface();

    // State
    bool running = false;

private:
    // Refresh Rate
    std::chrono::time_point<std::chrono::high_resolution_clock> prevSpectrumTime = std::chrono::high_resolution_clock::now();
    std::chrono::time_point<std::chrono::high_resolution_clock> currentSpectrumTime = prevSpectrumTime;

    // FFT Information
    qint64 bufferTime;

    Engine* m_engine;

    void getFFTStats();
    void setupConnections();
    void startRecording();
    void trackTime();

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
    void toggleMicrophoneSaving(bool state);

signals:
    // To UI
    void sendSpectrum(const FrequencySpectrum &spectrum);
    void sendRefreshRate(long long refreshRateUs);

    // To Engine
    void startEngineRecording();
    void pauseEngineRecording();
    void stopEngineRecording(); // dumps data to file and resets buffer
    void sendVisualizerGain(double); // pass to engine
    void resetBuffer();
    void toggleSavingData(bool state);

    // To UI & Engine
    void sendTargetFrequencies(int* frequencies, int* intensityThresholds);
};

#endif // FFTINTERFACE_H

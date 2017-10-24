#ifndef FLIR_H
#define FLIR_H

#include <QThread>

// Pv Stuff from eBUS SDK
#include <PvBuffer.h>
#include <PvBufferWriter.h>
#include <PvDevice.h>
#include <PvDeviceGEV.h>
#include <PvDeviceInfo.h>
#include <PvDeviceInfoU3V.h>
#include <PvDeviceU3V.h>
#include <PvSampleUtils.h>
#include <PvStream.h>
#include <PvStreamGEV.h>
#include <PvStreamU3V.h>
#include <PvTransmitterLib.h>
#include <PvTypes.h>

#include <QCoreApplication>
#include <QDateTime>
#include <QLabel>
#include <QDir>

// Conditional Inclusion?
#ifdef PV_GUI_NOT_AVAILABLE
#include <PvSystem.h>
#else
#include <PvDeviceFinderWnd.h>
#endif // PV_GUI_NOT_AVAILABLE

#include <list>
typedef std::list<PvBuffer *> BufferList;

#define BUFFER_COUNT ( 16 )

class flir : public QObject
{ Q_OBJECT
public:
    flir();
    ~flir();

    static const int flirFrameFileSize = 81920; // bytes

private:
    void acquireImages(PvDevice *aDevice, PvStream *aStream); // Fetches Images
    PvDevice *connectToDevice( const PvDeviceInfo *aDeviceInfo );
    PvStream *openStream( const PvDeviceInfo *aDeviceInfo );
    void configureStream( PvDevice *aDevice, PvStream *aStream );
    void createStreamBuffers(PvDevice *aDevice, PvStream *aStream, BufferList *aBufferList);
    void freeStreamBuffers( BufferList *aBufferList );
    void disconnectFLIR();

    // State Variables
    bool running;
    bool recording;
    QString threadName = "FLIR"; // for log messages

    int firstFrameID;
    int lastFrameID;

public slots:
    void startFLIRThread();
    void initAcquisition();
    void record(bool state);
    // void toggleHighTempMode(bool mode); // false = low T; true = high T

signals:
    void sendLogMessage(QString msg, QString msgType, QString threadID);
    void connection(bool state);
    void finished();
    void sendFrame(QPixmap);
    void sendFrameRate(double);
    void sendFrameCount(int);
    void sendFeedbackMessage(QString msg);
};

#endif // FLIR_H

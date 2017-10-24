#include "flir.h"

PV_INIT_SIGNAL_HANDLER()

flir::flir()
{
    // initial state
    running = false;
    recording = false;
}

flir::~flir()
{

}

void flir::initAcquisition() // begin image acquisition process
{
    sendLogMessage("Initializing FLIR Acquisition","DEBUG",threadName);

    PvDevice *lDevice = NULL;
    PvStream *lStream = NULL;
    BufferList lBufferList;
    running = true;

    PV_SAMPLE_INIT();
    PvSystem *lPvSystem = new PvSystem;
    PvResult lResult;

    lResult = lPvSystem->Find();
    if ( !lResult.IsOK() )
    {
        //sendLogMessage("PvSystem::Find Error: " + lResult.GetCodeString().GetAscii(),"ERROR",threadName);
        cout << "PvSystem::Find Error: " << lResult.GetCodeString().GetAscii();
    }

    //uint32_t lInterfaceCount = lPvSystem->GetInterfaceCount();


    // Get pointer to the interface.
    const PvInterface* lInterface = lPvSystem->GetInterface( 0 );

    uint32_t lDeviceCount = lInterface->GetDeviceCount();
    if(lDeviceCount == 0)
    {
        sendLogMessage("No FLIR Device Connected","ERROR",threadName);
        connection(false);
        return;
    }

    const PvDeviceInfo *lDeviceInfo = lInterface->GetDeviceInfo( 0 );

    //const PvDeviceInfoGEV* lDeviceInfoGEV = dynamic_cast<const PvDeviceInfoGEV*>( lDeviceInfo );

    if ( NULL != lDeviceInfo )
    {
        lDevice = connectToDevice( lDeviceInfo );
        if ( NULL != lDevice )
        {
            lStream = openStream( lDeviceInfo );
            if ( NULL != lStream )
            {
                configureStream( lDevice, lStream );
                createStreamBuffers( lDevice, lStream, &lBufferList );
                acquireImages( lDevice, lStream ); // ACTUALLY GETS IMAGES
                freeStreamBuffers( &lBufferList );

                // Close the stream
                cout << "Closing stream" << endl;
                lStream->Close();
                PvStream::Free( lStream );
            }
            else
                cout << "No Stream" << endl;

            // Disconnect the device
            cout << "Disconnecting device" << endl;
            lDevice->Disconnect();
            PvDevice::Free( lDevice );
        }
        else
            cout << "No Device" << endl;
    }
    else
        cout << "No Device Info" << endl;
    if( NULL != lPvSystem )
    {
        delete lPvSystem;
        lPvSystem = NULL;
    }

    PV_SAMPLE_TERMINATE();
}

PvDevice* flir::connectToDevice( const PvDeviceInfo *aDeviceInfo )
{
    PvDevice *lDevice;
    PvResult lResult;

    // Connect to the GigE Vision or USB3 Vision device
    cout << "Connecting to " << aDeviceInfo->GetDisplayID().GetAscii() << "." << endl;
    lDevice = PvDevice::CreateAndConnect( aDeviceInfo, &lResult );
    if ( lDevice == NULL )
    {
        cout << "Unable to connect to " << aDeviceInfo->GetDisplayID().GetAscii() << "." << endl;
    }
    return lDevice;
}

PvStream* flir::openStream( const PvDeviceInfo *aDeviceInfo )
{
    PvStream *lStream;
    PvResult lResult;

    // Open stream to the GigE Vision or USB3 Vision device
    cout << "Opening stream to device." << endl;
    lStream = PvStream::CreateAndOpen( aDeviceInfo->GetConnectionID(), &lResult );
    if ( lStream == NULL )
    {
        cout << "Unable to stream from " << aDeviceInfo->GetDisplayID().GetAscii() << "." << endl;
    }

    return lStream;
}

void flir::configureStream( PvDevice *aDevice, PvStream *aStream )
{
    cout << "Configuring Stream." << endl;
    // If this is a GigE Vision device, configure GigE Vision specific streaming parameters
    PvDeviceGEV* lDeviceGEV = dynamic_cast<PvDeviceGEV *>( aDevice );
    if ( lDeviceGEV != NULL )
    {
        PvStreamGEV *lStreamGEV = static_cast<PvStreamGEV *>( aStream );

        // Negotiate packet size
        lDeviceGEV->NegotiatePacketSize();

        // Configure device streaming destination
        lDeviceGEV->SetStreamDestination( lStreamGEV->GetLocalIPAddress(), lStreamGEV->GetLocalPort() );
    }
}

void flir::createStreamBuffers( PvDevice *aDevice, PvStream *aStream, BufferList *aBufferList )
{
    cout << "Creating Stream Buffers." << endl;
    // Reading payload size from device
    uint32_t lSize = aDevice->GetPayloadSize();

    // Use BUFFER_COUNT or the maximum number of buffers, whichever is smaller
    uint32_t lBufferCount = ( aStream->GetQueuedBufferMaximum() < BUFFER_COUNT ) ?
        aStream->GetQueuedBufferMaximum() :
        BUFFER_COUNT;

    // Allocate buffers
    for ( uint32_t i = 0; i < lBufferCount; i++ )
    {
        // Create new buffer object
        PvBuffer *lBuffer = new PvBuffer;

        // Have the new buffer object allocate payload memory
        lBuffer->Alloc( static_cast<uint32_t>( lSize ) );

        // Add to external list - used to eventually release the buffers
        aBufferList->push_back( lBuffer );
    }

    // Queue all buffers in the stream
    BufferList::iterator lIt = aBufferList->begin();
    while ( lIt != aBufferList->end() )
    {
        aStream->QueueBuffer( *lIt );
        lIt++;
    }
}

void flir::acquireImages( PvDevice *aDevice, PvStream *aStream )
{
    cout << "Acquiring Images." << endl;
    // Get device parameters need to control streaming
    PvGenParameterArray *lDeviceParams = aDevice->GetParameters();

    // Map the GenICam AcquisitionStart and AcquisitionStop commands
    PvGenCommand *lStart = dynamic_cast<PvGenCommand *>( lDeviceParams->Get( "AcquisitionStart" ) );
    PvGenCommand *lStop = dynamic_cast<PvGenCommand *>( lDeviceParams->Get( "AcquisitionStop" ) );

    // Get stream parameters
    PvGenParameterArray *lStreamParams = aStream->GetParameters();

    // Map a few GenICam stream stats counters
    PvGenFloat *lFrameRate = dynamic_cast<PvGenFloat *>( lStreamParams->Get( "AcquisitionRate" ) );
    PvGenFloat *lBandwidth = dynamic_cast<PvGenFloat *>( lStreamParams->Get( "Bandwidth" ) );

    // Enable streaming and send the AcquisitionStart command
    cout << "Enabling streaming and sending AcquisitionStart command." << endl;
    aDevice->StreamEnable();
    lStart->Execute();

    double lFrameRateVal = 0.0;
    double lBandwidthVal = 0.0;
    int id;

    bool firstLoop = true; // used to get first frame number

    // Create char* path for saving images
    char path[]= "/home/mattlamsey/Documents/Sensor GUI/build-SensorGUI_1-Desktop_Qt_5_9_1_GCC_64bit-Debug/";
    const QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString(QLatin1String("MM-dd.hh:mm"));
    QByteArray tarray = timestamp.toLocal8Bit();
    const char *stamp = tarray.data();
    strcat(path,"FLIR_Images/");
    strcat(path, stamp);
    strcat(path, "/");
    QDir dir;

    // copy the path to be modified for each frame
    char* filename = strcpy(filename,path);

    connection(true); // signal
    // Acquire images until the user instructs us to stop.
    while ( running )
    {
        PvBuffer *lBuffer = NULL;
        PvResult lOperationResult;
        PvBufferWriter lBufferWriter;

        // Retrieve next buffer
        PvResult lResult = aStream->RetrieveBuffer( &lBuffer, &lOperationResult, 1000 );
        if ( lResult.IsOK() )
        {
            if ( lOperationResult.IsOK() )
            {
                PvPayloadType lType;

                lFrameRate->GetValue( lFrameRateVal );
                lBandwidth->GetValue( lBandwidthVal );

                // If the buffer contains an image, display width and height.
                uint32_t lWidth = 0, lHeight = 0;
                lType = lBuffer->GetPayloadType();

                cout << fixed << setprecision( 1 );
                if ( lType == PvPayloadTypeImage )
                {
                    // Get image specific buffer interface.
                    PvImage *lImage = lBuffer->GetImage();


                    // Read width, height.
                    lWidth = lImage->GetWidth();
                    lHeight = lImage->GetHeight();

                    lBuffer->GetImage()->Alloc(lWidth, lHeight, lBuffer->GetImage()->GetPixelType());

                    // SAVE IMAGES
                    if (recording)
                    {
                        filename = strcpy(filename,path);
                        if (!dir.exists(path))
                            dir.mkpath(path);

                        if (lBuffer->GetBlockID()%2==0)
                        {
                            id = lBuffer->GetBlockID();
                            std::string s=std::to_string(id);
                            if(firstLoop){
                                firstFrameID = id/2;
                                lastFrameID = firstFrameID;
                                cout << "First Frame: ";
                                cout << firstFrameID << endl;
                                firstLoop = false;
                            }
                            else
                                lastFrameID = id/2;
                            char const *schar=s.c_str();

                            strcat(filename, schar);
                            strcat(filename,".raw");
                            lBufferWriter.Store(lBuffer,filename,PvBufferFormatRaw);
                            sendFrameCount(lastFrameID-firstFrameID);
                        }
                    }

                    // Display Image
                    unsigned char* Data = lImage->GetDataPointer();
                    QImage myImage(Data, 320, 256, QImage::Format_Indexed8);
                    sendFrame(QPixmap::fromImage(myImage)); // signal to main
                }
                else
                {
                    sendLogMessage("(buffer does not contain image)","ERROR",threadName);
                }
                sendFrameRate(lFrameRateVal); // signal to main
            }
            // Re-queue the buffer in the stream object
            aStream->QueueBuffer( lBuffer );
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
    // Tell the device to stop sending images.
    cout << "Sending AcquisitionStop command to the device" << endl;
    lStop->Execute();

    // Disable streaming on the device
    cout << "Disable streaming on the controller." << endl;
    aDevice->StreamDisable();

    // Abort all buffers from the stream and dequeue
    cout << "Aborting buffers still in stream" << endl;
    aStream->AbortQueuedBuffers();
    while ( aStream->GetQueuedBufferCount() > 0 )
    {
        PvBuffer *lBuffer = NULL;
        PvResult lOperationResult;

        aStream->RetrieveBuffer( &lBuffer, &lOperationResult );
    }
}

void flir::freeStreamBuffers( BufferList *aBufferList )
{
    // Go through the buffer list
    BufferList::iterator lIt = aBufferList->begin();
    while ( lIt != aBufferList->end() )
    {
        delete *lIt;
        lIt++;
    }

    // Clear the buffer list
    aBufferList->clear();
}

// ----- SLOTS ----- //
void flir::startFLIRThread()
{
    sendLogMessage("FLIR Thread Initialized.","SYSTEM",threadName);
    sendFeedbackMessage("FLIR device not found.");
}

void flir::disconnectFLIR()
{
    emit finished();
}

void flir::record(bool state)
{
    recording = state;
}

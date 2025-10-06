#include "QCoreApplication"
#include <QCommandLineParser>
#include <QDebug>

// Test Logging
//#include "logging/BasicLogging.cpp"

// Test Signals & Slots
//#include "signals-slots/observer.h"
//#include "signals-slots/reporter.h"

// Aufgaben
#include "source/transceiver/client.h"
#include "source/transceiver/audiomanager.h"
#include "source/transceiver/portAudioCallback.h"
#include "source/transceiver/utils.h"

// --- Build from Terminal: ---
// after editing the .pro-file: run 'qmake ias.pro' followed by 'make'
// after only editing some code: run 'make'
// --- Start from Terminal: ---
// build and then run with './ias -i 127.0.0.1 -p 5041'

// flag | description | values
// -i | IP |  ip-adresse
// -p | port | port nummer
// -e | encoding | true/false
// -m | input device (microphone) index| int
// -l | output device (lautsprecher) index | int
// -j | jitter buffer size | int | default: 50
// -f | frames per buffer (frame size) | int
// -s | sample rate | int | default
// -c | audio channels | int | default: 1

int main(int argc, char *argv[])
{
    // Creates an Event-Loop for de Application without an Interface
    QCoreApplication a(argc, argv);

    // Create a CommandLineParser so we can enter IP and Port as command line arguments
    QCommandLineParser parser;

    QCommandLineOption optIp = {{"i", "ip"}, "IP-Address", "ip"};
    optIp.setDefaultValue("127.0.0.1");

    QCommandLineOption optPort = {{"p", "port"}, "Port Number", "port"};
    optPort.setDefaultValue("5401");

    QCommandLineOption optOpusEncoding = {{"e", "encoded"}, "Opus-Encoding true/false", "encoded"};
    optOpusEncoding.setDefaultValue("true");

    QCommandLineOption optInDev = {{"m", "inDev"}, "Input device index (microphone)", "inDeviceIndex"};
    optInDev.setDefaultValue("0");

    QCommandLineOption optOutDev = {{"l", "outDev"}, "Output device index (speaker)", "outDeviceIndex"};
    optOutDev.setDefaultValue("1");

    QCommandLineOption optJitterBufferSize = {{"j", "Jitter Buffer Size"}, "Size for the Jitter Buffer as Integer", "jitterBufferSize"};
    optOpusEncoding.setDefaultValue("50");

    QCommandLineOption optFramesPerBuffer = {{"f", "fpb"}, "Frames per buffer", "framesPerBuffer"};
    optFramesPerBuffer.setDefaultValue("512");

    QCommandLineOption optSampleRate = {{"s", "sr"}, "Sample rate", "sampleRate"};
    optSampleRate.setDefaultValue("48000");

    QCommandLineOption optAudioChannels = {{"c", "ac"}, "Audio channels", "audioChannels"};
    optAudioChannels.setDefaultValue("1");


    parser.setApplicationDescription("Start audio callback function that sends input to server.");
    parser.addHelpOption();
    parser.addOptions({optOpusEncoding, optIp, optPort, optJitterBufferSize, optInDev, optOutDev, optFramesPerBuffer, optSampleRate, optAudioChannels});
    parser.process(a);

    const QString ipIn = parser.value(optIp);
    const QString portIn = parser.value(optPort);
    const QString encodingEnabled = parser.value(optOpusEncoding);
    const QString inputDeviceIndex = parser.value(optInDev);
    const QString outputDeviceIndex = parser.value(optOutDev);
    const QString jitterBufferSize = parser.value(optJitterBufferSize);
    const QString framesPerBuffer = parser.value(optFramesPerBuffer);
    const QString sampleRate = parser.value(optSampleRate);
    const QString audioChannels = parser.value(optAudioChannels);

    //qDebug() << "Ip: " << ipIn << ", Port: " << portIn;

    /// Aufgabe A
    // Create Instance of Client and run it.
    /*Client myClient(nullptr);
    if(myClient.InitializeWithTimer(ipIn, portIn))
    {
        myClient.RunWithTimer();
    }*/

    /// Aufgabe B
    AudioManager audioManager;
    if (audioManager.Initialize(
        encodingEnabled,
        framesPerBuffer,
        sampleRate,
        jitterBufferSize,
        audioChannels,
        inputDeviceIndex,
        outputDeviceIndex,
        ipIn,
        portIn))
    {
        audioManager.StartAudioStream();
    }


//
    /// Signals + Slots
    //Reporter reporter;
    //Observer observer;
    //observer.subscribeToEvent(&reporter);
    //QObject::connect(&observer, &Observer::onObserve, &a, &QCoreApplication::quit, Qt::QueuedConnection);
    //reporter.sendSignal();

    return a.exec();
}


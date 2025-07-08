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

int main(int argc, char *argv[])
{
    // Creates an Event-Loop for de Application without an Interface
    QCoreApplication a(argc, argv);

    // Create a CommandLineParser so we can enter IP and Port as command line arguments
    QCommandLineParser parser;

    QCommandLineOption optIp = {{"i", "ip"}, "IP-Address" };
    optIp.setDefaultValue("127.0.0.1");

    QCommandLineOption optPort = {{"p", "port"}, "Port Number"};
    optPort.setDefaultValue("5401");

    QCommandLineOption optFramesPerBuffer = {{"f", "fpb"}, "frames per buffer"};
    optFramesPerBuffer.setDefaultValue("512");

    QCommandLineOption optSampleRate = {{"s", "sr"}, {"sample rate"}};
    optSampleRate.setDefaultValue("48000");

    QCommandLineOption optAudioChannels = {{"c", "ac"}, "audio channels"};
    optAudioChannels.setDefaultValue("1");

    QCommandLineOption optInDev = {{"in", "inDev"}, "Inputdevice index"};
    optInDev.setDefaultValue("0");

    QCommandLineOption optOutDev = {{"out", "outDev"}, "Outputdevice index"};
    optOutDev.setDefaultValue("1");

    parser.setApplicationDescription("Start audio callback function that sends input to server.");
    parser.addHelpOption();
    parser.addOptions({optIp, optPort, optFramesPerBuffer, optSampleRate, optAudioChannels, optInDev, optOutDev});
    parser.process(a);

    const QString ipIn = parser.value(optIp);
    const QString portIn = parser.value(optPort);
    const QString framesPerBuffer = parser.value(optFramesPerBuffer);
    const QString sampleRate = parser.value(optSampleRate);
    const QString audioChannels = parser.value(optAudioChannels);
    QString inputDeviceIndex = parser.value(optInDev);
    QString outputDeviceIndex = parser.value(optOutDev);

    qDebug() << "Ip: " << ipIn << ", Port: " << portIn;

    /// Aufgabe A
    // Create Instance of Client and run it.
    // Client myClient(nullptr);
    //if(myClient.InitializeWithTimer(ipIn, portIn))
    //{
    //    myClient.RunWithTimer();
    //}

    /// Aufgabe B
    AudioManager audioManager;
    if (audioManager.Initialize(framesPerBuffer, sampleRate, audioChannels, ipIn, portIn))
    {
        audioManager.StartAudioStream();
    }

    /// Signals + Slots
    //Reporter reporter;
    //Observer observer;
    //observer.subscribeToEvent(&reporter);
    //QObject::connect(&observer, &Observer::onObserve, &a, &QCoreApplication::quit, Qt::QueuedConnection);
    //reporter.sendSignal();

    return a.exec();
}


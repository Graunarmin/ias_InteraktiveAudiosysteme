#include "QCoreApplication"
#include "QUdpSocket"
#include <QCommandLineParser>
#include <QDebug>

// Test Logging
//#include "logging/BasicLogging.cpp"

// Test Signals & Slots
//#include "signals-slots/observer.h"
//#include "signals-slots/reporter.h"

#include "source/transceiver/server.h"

// --- Build in Terminal: ---
// after editing the .pro-file: run 'qmake' followed by 'make'
// after only editing some code: run 'make'
// --- Run in Terminal: ---
// build and then run with './ias'

int main(int argc, char *argv[])
{
    // Creates an Event-Loop for de Application without an Interface
    QCoreApplication a(argc, argv);

    QString ipIn = "127.0.0.1";
    QString portIn = "5401";

    QCommandLineParser parser;

    QCommandLineOption optIp = {{"i", "ip"}, "IP-Address", ipIn};
    QCommandLineOption optPort = {{"p", "port"}, "Port Number", portIn};

    parser.setApplicationDescription("Send Data to Server");
    parser.addHelpOption();
    parser.addOptions({optIp, optPort});
    parser.process(a);

    //const QStringList args = parser.positionalArguments();

    if(argc > 1)
    {
        if(parser.isSet(optIp) && !parser.value(optIp).isEmpty())
        {
            ipIn = parser.value(optIp);
        }
        if(parser.isSet(optPort) && !parser.value(optPort).isEmpty())
        {
            portIn = parser.value(optPort);
        }
    }

    qDebug() << "Ip: " << ipIn << ", Port: " << portIn;

    //Reporter reporter;
    //Observer observer;
    //observer.subscribeToEvent(&reporter);
    //QObject::connect(&observer, &Observer::onObserve, &a, &QCoreApplication::quit, Qt::QueuedConnection);
    //reporter.sendSignal();

    MyClient myClient(nullptr);
    if(myClient.Initialize(ipIn, portIn))
    {
        myClient.Run();
    }

    return a.exec();
}


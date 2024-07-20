#include "QCoreApplication"
#include "QUdpSocket"

// Test Logging
#include "logging/BasicLogging.cpp"

// Test Signals & Slots
#include "signals-slots/observer.h"
#include "signals-slots/reporter.h"

// --- Build in Terminal: ---
// after editing the .pro-file: run 'qmake' followed by 'make'
// after only editing some code: run 'make'
// --- Run in Terminal: ---
// build and then run with './ias'

int main(int argc, char *argv[])
{
    // Creates an Event-Loop for de Application without an Interface
    QCoreApplication a(argc, argv);

    Reporter reporter;
    Observer observer;
    observer.subscribeToEvent(&reporter);

    QObject::connect(&observer, &Observer::onObserve, &a, &QCoreApplication::quit, Qt::QueuedConnection);

    reporter.sendSignal();

    return a.exec();
}


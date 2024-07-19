#include "QCoreApplication"
#include "QUdpSocket"

#include "BasicLogging.cpp"

int main(int argc, char *argv[])
{
    // Creates an Event-Loop for de Application without an Interface
    QCoreApplication a(argc, argv);

    //doCpp();
    //doQt();
    doMixed();

    return a.exec();
}


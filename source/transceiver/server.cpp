#include "server.h"

/*
 * Wir wollen via UDP Daten an einen Client (Reflektor) schicken, der unter der IP-Adresse 144.76.81.210
 * und dem Port 5401 erreichbar ist.
 * Verschickt werden sollen im Sekundentakt die Zahlen von 1 bis 10 in einer Endlosschleife.
 * Das Rückempfangen jedes Wertes soll mit einer Konsolenausgabe „Wert x empfangen“ quittiert werden.
 */

struct MyClient::Impl
{
    std::shared_ptr<QUdpSocket> spUdpSocket = {std::make_shared<QUdpSocket>()};
    QHostAddress ip = {QHostAddress()};
    quint16 port = {0};
    std::shared_ptr<QTimer> spTimer = {std::make_shared<QTimer>()};
    int timesRun = {0};
    int data = {0};

    Impl(){}
};

MyClient::MyClient(QObject *parent)
    : QObject{parent}
    , m(std::make_shared<Impl>())
{ }

bool MyClient::Initialize(QString ipIn, QString portIn)
{
    //QString ipIn;
   // QString portIn;

    // do
    // {
    //     readInPort(ipIn, portIn);
    // }
    // while(!verifyParameters(ipIn, portIn, m->ip, m->port));

    bool success = false;

    if(verifyParameters(ipIn, portIn, m->ip, m->port))
    {
        /// Create a timer with an interval of a certain amaount of miliseconds.
        /// Then connect the timeout of the timer to the sendData function.
        /// This way every intervall the sendDate Function will be called.
        m->spTimer -> setInterval(1000);

        // connect timer to sending data
        connect(m->spTimer.get(), &QTimer::timeout, this, &MyClient::slotSendData);

        // connect socket-receive to confirmation function
        connect(m->spUdpSocket.get(), &QUdpSocket::readyRead, this, &MyClient::slotReceivedReflectedData);
        success = true;
    }
    return success;
}

void MyClient::Run()
{
    m->spTimer -> start();
    qInfo() << "Trying to send Data to port " << m->port << " at " << m->ip;
}



void MyClient::readInPort(QString& ipIn, QString& portIn)
{
    QTextStream qin(stdin);
    qInfo() << "Please enter the Port Number: ";
    portIn = qin.readLine();

    qInfo() << "An now the IP-Address: ";
    ipIn = qin.readLine();
}

bool MyClient::verifyParameters(QString ipIn, QString portIn, QHostAddress& ipOut, quint16& portOut)
{
    bool success = true;
    bool inputWasNumber = false;
    int port = portIn.toInt(&inputWasNumber);

    if(inputWasNumber){
        if(port > 0 && port <= 0xffff)
        {
            portOut = static_cast<quint16>(port);
        }
        else
        {
            qInfo() << "Not a valid port.";
            success = false;
        }
    }
    else
    {
        qInfo() << "Not a valid port.";
        success = false;
    }

    QHostAddress ip = QHostAddress(ipIn);
    if(ip.isNull())
    {
        qInfo() << "Not a valid IP-Address.";
        success = false;
    }
    else
    {
        ipOut = ip;
    }

    return success;
}


/*!
 * \brief MyClient::SendData
 * Send Data to MyClient
 */
void MyClient::slotSendData()
{
    if(m->data < 10)
    {
        m->data += 1;
    }
    else
    {
        m->data = 1;
    }

    QByteArray baData = QByteArray::fromRawData(reinterpret_cast<const char *>(&(m->data)), sizeof(m->data));
    qDebug() << "Outgoing Byte Array: " << baData;
    qint64 sentBytes = m->spUdpSocket -> writeDatagram(baData, m->ip, m->port);

    int checkNumber = static_cast<int>(sentBytes);
    if(checkNumber < 0)
    {
        qInfo() << "There was an Error while sending Data to the Server.";
    }
    else
    {
        qInfo() << "Successfully sent " << checkNumber << "Bytes of data with value " << m->data << " to Server.";
    }
}

void MyClient::slotReceivedReflectedData()
{
    while (m->spUdpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m->spUdpSocket->receiveDatagram();
        QByteArray baData = datagram.data();
        qDebug() << "Incoming Byte Array: " << baData;
        qInfo() << "Received Data of size " << baData.size() << " with value " << baData.toInt();
    }
}







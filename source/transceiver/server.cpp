#include "server.h"

/*
 * Wir wollen via UDP Daten an einen Server (Reflektor) schicken, der unter der IP-Adresse 144.76.81.210
 * und dem Port 5401 erreichbar ist.
 * Verschickt werden sollen im Sekundentakt die Zahlen von 1 bis 10 in einer Endlosschleife.
 * Das Rückempfangen jedes Wertes soll mit einer Konsolenausgabe „Wert x empfangen“ quittiert werden.
 */

Server::Server(QObject *parent) : QObject(parent)
{
    _udpSocket = new QUdpSocket(this);
    int port = -1;
    while(port < 0)
    {
        QString portStr = readInPort();
        port = checkPortValidity(portStr);
    }
    qInfo() << "Sending Data to port" << port;
    _port = port;
    initSocket();
    SendData();
}

Server::~Server()
{

}

void Server::initSocket()
{
    // bind socket to an adress and port
    _udpSocket -> bind(QHostAddress::LocalHost, _port);

    // connect the sender and it's signal to a receiver and it's slot
    // connect the udpSockets readyRead signal to this classes readPendingDatagrams-Slot
    // so whenever the socket receives Data, our readPendingDatagrams function is called.
    connect(_udpSocket, &QUdpSocket::readyRead,
            this, &Server::readPendingDatagrams);
}

QString Server::readInPort()
{
    QTextStream qin(stdin);
    qInfo() << "Please enter the Port Number: ";
    return qin.readLine();
}

int Server::checkPortValidity(QString input)
{
    bool inputWasNumber = false;
    int port = input.toInt(&inputWasNumber);

    if(inputWasNumber){
        if(port > 0)
        {
            return port;
        }
    }
     qInfo() << "Not a valid port.";
    return -1;
}

/*!
 * \brief Server::SendData
 * Send Data to Server
 */
void Server::SendData()
{
    int data;
    for (int i = 1; i < 11; ++i)
    {
        data = i;
        qInfo() << data;

        //_udpSocket -> writeDatagram(data, QHostAddress::LocalHost, )
        //emit intDataSignal(data);
    }
}

/*!
 * \brief Server::readPendingDatagrams
 * Read Data
 */
void Server::readPendingDatagrams()
{
    // while(_udpSocket -> hasPendingDatagrams()){
    //     QHostAddress *receivedAddress = NULL;
    //     quint16 *receivedPort = NULL;

    // }
}







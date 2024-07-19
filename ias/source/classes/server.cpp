#include "server.h"

/*
 * Wir wollen via UDP Daten an einen Server (Reflektor) schicken, der unter der IP-Adresse 144.76.81.210
 * und dem Port 5401 erreichbar ist.
 * Verschickt werden sollen im Sekundentakt die Zahlen von 1 bis 10 in einer Endlosschleife.
 * Das Rückempfangen jedes Wertes soll mit einer Konsolenausgabe „Wert x empfangen“ quittiert werden.
 */

Server::Server()
{
    udpSocket = new QUdpSocket(this);
}

Server::Server(QObject *parent) : QObject(parent)
{
    initSocket();
}

Server::~Server()
{

}

void Server::initSocket()
{
    udpSocket = new QUdpSocket(this);
    udpSocket -> bind(QHostAddress::LocalHost, 5401);

    connect(udpSocket, &QUdpSocket::readyRead,
            this, &Server::readPendingDatagrams);
}

void Server::readPendingDatagrams()
{
    while(udpSocket -> hasPendingDatagrams()){
        QHostAddress *receivedAddress = NULL;
        quint16 *receivedPort = NULL;

    }
}






